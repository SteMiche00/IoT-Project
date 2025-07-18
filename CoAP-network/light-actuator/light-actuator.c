#include "contiki.h"
#include "coap-engine.h"
#include "coap-observe-client.h"
#include "coap-blocking-api.h"
#include "dev/leds.h"
#include "os/dev/button-hal.h"
#include <stdio.h>
#include <string.h>
#include "dev/serial-line.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"

#include "../ml-classifier/room_occupancy_forecast.h"

#define LOG_MODULE "LightActuator"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_EP "coap://[fd00::1]/registration"
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)
#define DISCOVERY_EP "coap://[fd00::1]/sensors-discovery"
#define OBSERVE_INTERVAL 30

static char* service_name = "actuator_light";

static coap_endpoint_t server_ep;
static coap_message_t request[1];
static coap_observee_t *obs[3];

static float min_light = 200.0;
static int status = 0; 

static float room_data[3] = {0, 0, 0}; // 0=temp, 1=light, 2=humidity
static float room_occupancy_probability[2] = {0, 0};

static bool is_registered = false;
static char payload[128];
static bool sensors_discovered = false;
static bool first_value_observed[3] = {false, false, false};
static bool energy_saving = false;

static char sensor_urls[3][128];  // parsed IPs + resource paths
static const char *sensor_types[] = { "temp", "light", "humidity" };
static char *resource_paths[] = {
  "sensors/temp",
  "sensors/light",
  "sensors/humidity"
};

static bool sensors_found[3] = {false, false, false};  // temp, light, humidity

static void res_post_threshold_handler(coap_message_t *request,
                                       coap_message_t *response,
                                       uint8_t *buffer,
                                       uint16_t preferred_size,
                                       int32_t *offset);

static void res_get_threshold_handler(coap_message_t *request,
                                       coap_message_t *response,
                                       uint8_t *buffer,
                                       uint16_t preferred_size,
                                       int32_t *offset);
static void res_get_status_handler(coap_message_t *request,
                                       coap_message_t *response,
                                       uint8_t *buffer,
                                       uint16_t preferred_size,
                                       int32_t *offset);
                                
RESOURCE(res_thresholds,
         "title=\"Thresholds\";rt=\"Control\"",
         res_get_threshold_handler,  // GET 
         res_post_threshold_handler, // POST
         NULL,                      // PUT
         NULL);                     // DELETE

RESOURCE(res_status,
         "title=\"Status\";rt=\"Control\";obs",
         res_get_status_handler,  // GET 
         NULL,                      // POST
         NULL,                      // PUT
         NULL);                     // DELETE

static void res_post_threshold_handler(coap_message_t *request,
                                       coap_message_t *response,
                                       uint8_t *buffer,
                                       uint16_t preferred_size,
                                       int32_t *offset) {
  size_t len = coap_get_payload(request, (const uint8_t **)&buffer);
  buffer[len] = '\0';

  printf("[ACTUATOR LIGHT] Received CoAP POST: %s\n", buffer);

  int new_min;
  if (sscanf((char *)buffer, "min=%d", &new_min) == 1) {
    min_light = new_min / 1000.0; // Convert to float
    printf("[ACTUATOR LIGHT] Updated thresholds: min_light=%.3f\n",
           min_light);
    coap_set_status_code(response, CHANGED_2_04);
  } else {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}

static void res_get_threshold_handler(coap_message_t *request,
                                      coap_message_t *response,
                                      uint8_t *buffer,
                                      uint16_t preferred_size,
                                      int32_t *offset){
  int len = snprintf((char *)buffer, preferred_size,
                     "min=%d",
                     (int)(min_light * 1000));

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static void res_get_status_handler(coap_message_t *request,
                                      coap_message_t *response,
                                      uint8_t *buffer,
                                      uint16_t preferred_size,
                                      int32_t *offset){
  int len = snprintf((char *)buffer, preferred_size,
                     "%d",
                     status);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static bool is_connected() {
  if(NETSTACK_ROUTING.node_is_reachable()) {
    printf("[ACTUATOR LIGHT] Connected to Border Router\n");
    return true;
  } else {
    printf("[ACTUATOR LIGHT] Waiting for connection with the Border Router\n");
  }
  return false;
}

static void registration_response_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR LIGHT] Registration request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(strncmp((char*)chunk, "Registered", len) == 0)
    is_registered = true;
  else
    is_registered = false;

  printf("[ACTUATOR LIGHT] Registration response: %.*s\n", len, (char *)chunk);
}

static int uri_to_index(const char *uri) {
  if(strcmp(uri, "sensors/temp") == 0) {
    return 0;
  } else if(strcmp(uri, "sensors/light") == 0) {
    return 1;
  } else if(strcmp(uri, "sensors/humidity") == 0) {
    return 2;
  } else {
    return -1; // Valore di default per URI non riconosciuti
  }
}

static void discovery_parser(const uint8_t *chunk, int len) {
  char buffer[256];  
  int copy_len = len < (int)(sizeof(buffer) - 1) ? len : (int)(sizeof(buffer) - 1);

  memcpy(buffer, chunk, copy_len);
  buffer[copy_len] = '\0';

  printf("[ACTUATOR LIGHT] Received discovery payload: %s\n", buffer);

  if(buffer[copy_len - 1] == ';') {
    buffer[copy_len - 1] = '\0';
  }

  char type[32] = {0};
  char ip[96] = {0};

  if(sscanf(buffer, "sensor_%31[^@]@%95[^-]", type, ip) == 2) {
    printf("[ACTUATOR LIGHT] Parsed sensor type='%s' ip='%s'\n", type, ip);

    if(strcmp(type, "temp") == 0) {
      snprintf(sensor_urls[0], sizeof(sensor_urls[0]), "coap://[%s]", ip);
      sensors_found[0] = true;
    } else if(strcmp(type, "light") == 0) {
      snprintf(sensor_urls[1], sizeof(sensor_urls[1]), "coap://[%s]", ip);
      sensors_found[1] = true;
    } else if(strcmp(type, "humidity") == 0) {
      snprintf(sensor_urls[2], sizeof(sensor_urls[2]), "coap://[%s]", ip);
      sensors_found[2] = true;
    } else {
      printf("[ACTUATOR LIGHT] Unknown sensor type: %s\n", type);
    }
  } else {
    printf("[ACTUATOR LIGHT] Parsing failed for discovery payload\n");
  }

  if(sensors_found[0] && sensors_found[1] && sensors_found[2]) {
    sensors_discovered = true;
    printf("[ACTUATOR LIGHT] All sensors discovered:\n  Temp: %s\n  Light: %s\n  Humidity: %s\n",
           sensor_urls[0], sensor_urls[1], sensor_urls[2]);
  } else {
    printf("[ACTUATOR LIGHT] Waiting for more sensors to be discovered...\n");
  }
}

static void discovery_response_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR LIGHT] Sensor discovery timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(len <= 0) {
    printf("[ACTUATOR LIGHT] Sensor discovery failed: empty payload\n");
    return;
  }

  discovery_parser(chunk, len);
}

static void
notification_callback(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag) {
  int len = 0;
  const uint8_t *payload = NULL;

  printf("[ACTUATOR LIGHT] Observee URI: %s\n", obs->url);
  int sensor_index = uri_to_index(obs->url);

  switch(flag) {
    case NOTIFICATION_OK:
      printf("[ACTUATOR LIGHT] NOTIFICATION OK: %*s\n", len, (char *)payload);
      break;
    case OBSERVE_OK: /* server accepeted observation request */
      printf("[ACTUATOR LIGHT] OBSERVE_OK: %*s\n", len, (char *)payload);
      break;
    case OBSERVE_NOT_SUPPORTED:
      printf("[ACTUATOR LIGHT] OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *)payload);
      obs = NULL;
      break;
    case ERROR_RESPONSE_CODE:
      printf("[ACTUATOR LIGHT] ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[ACTUATOR LIGHT] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
  }

  if(notification) {
    len = coap_get_payload(notification, &payload);

    if (len > 0) {
      char value_str[16];
      memcpy(value_str, payload, len);
      value_str[len] = '\0';
      float value = atof(value_str) / 1000;

      first_value_observed[sensor_index] = true;
      room_data[sensor_index] = value;

      if(!first_value_observed[0] || !first_value_observed[1] || !first_value_observed[2]){
        printf("[ACTUATOR LIGHT] Room data not complete\n");
        return;
      }

      printf("[ACTUATOR LIGHT] Room data: %.3f C; %.3f lux; %.3f%%;\n", room_data[0], room_data[1], room_data[2]);
      eml_net_predict_proba(&room_occupancy_forecast, room_data, 3, room_occupancy_probability, 2);
      if (room_occupancy_probability[0] > room_occupancy_probability[1]){
        printf("[ACTUATOR LIGHT] Not occupied: %.3f%% - Occupied: %.3f%% - Status: Not occupied\n", room_occupancy_probability[0]*100, room_occupancy_probability[1]*100);
        if(energy_saving){
          printf("[ACTUATOR LIGHT] Energy saving mode: ON - Device OFF\n");
          leds_off(LEDS_RED);
          status = 0;
          return;
        }
      } else {
        printf("[ACTUATOR LIGHT] Not occupied: %.3f%% - Occupied: %.3f%% - Status: Occupied\n", room_occupancy_probability[0]*100, room_occupancy_probability[1]*100);
      }

      if(sensor_index == 0) {
        printf("[ACTUATOR LIGHT] Temperature: %.3f C\n", value);
      } else if(sensor_index == 1) {
        printf("[ACTUATOR LIGHT] Light: %.3f lux\n", value);

        if(value < min_light) {
          leds_on(LEDS_RED);
          status = 1;
          printf("[ACTUATOR LIGHT] Light LOW - Light ON (RED LED)\n");
        } else {
          leds_off(LEDS_RED);
          status = 0;
          printf("[ACTUATOR LIGHT] Light OK - Light OFF\n");
        } 
      } else if(sensor_index == 2) {
        printf("[ACTUATOR LIGHT] Humidity: %.3f %%\n", value);
      }
    }
  }
}

void toggle_observation(const int index){
  if(obs[index]) {
    printf("[ACTUATOR LIGHT] Stopping observation of %s\n", resource_paths[index]);
    coap_obs_remove_observee(obs[index]);
    obs[index] = NULL;
  } else {
    printf("[ACTUATOR LIGHT] Sending observe request to: %s\n", resource_paths[index]);
    
    coap_endpoint_t endpoint;
    coap_endpoint_parse(sensor_urls[index], strlen(sensor_urls[index]), &endpoint);

    obs[index] = coap_obs_request_registration(&endpoint, resource_paths[index], notification_callback, NULL);
  }
}

PROCESS(light_actuator, "Light Actuator");
AUTOSTART_PROCESSES(&light_actuator);

PROCESS_THREAD(light_actuator, ev, data)
{
  static bool es_led_state = false;
  static struct etimer reg_timer;
  static struct etimer connectivity_timer;
  static struct etimer et;
  static struct etimer led_timer;
  button_hal_button_t *button;

  PROCESS_BEGIN();

  coap_engine_init();
  coap_activate_resource(&res_thresholds, "actuators/light_th");
  coap_activate_resource(&res_status, "actuators/light_status");
  res_status.flags |= IS_OBSERVABLE;

  printf("[ACTUATOR LIGHT] Starting CoAP Light Actuator\n");
  printf("[ACTUATOR LIGHT] Light threshold: min=%.3f lux\n", min_light);
  printf("[ACTUATOR LIGHT] Dummy call to avoid warnings: %p, %s\n", eml_net_activation_function_strs, eml_error_str(0));

  button = button_hal_get_by_index(0);
  if(button){
    printf("%s on pin %u with ID=0, Logic=%s, Pull=%s\n",
    BUTTON_HAL_GET_DESCRIPTION(button), button->pin,
    button->negative_logic ? "Negative" : "Positive",
    button->pull == GPIO_HAL_PIN_CFG_PULL_UP ? "Pull Up" : "Pull Down");
  }

  while(!is_connected()) {
    etimer_set(&connectivity_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_UNTIL(etimer_expired(&connectivity_timer));
  }

  while(!is_registered) {
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
    coap_set_header_uri_path(request, "registration");
    snprintf(payload, sizeof(payload), "%s", service_name);
    coap_set_payload(request, (uint8_t *)payload, strlen(payload));

    printf("[ACTUATOR LIGHT] Sending registration for %s\n", service_name);
    COAP_BLOCKING_REQUEST(&server_ep, request, registration_response_handler);

    etimer_set(&reg_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_UNTIL(etimer_expired(&reg_timer));
  }

  // Sensor discovery with query parameters
  while(!sensors_discovered) {
    for(int i = 0; i < 3; i++) {
      if(!sensors_found[i]) {
        coap_endpoint_parse(DISCOVERY_EP, strlen(DISCOVERY_EP), &server_ep);
        coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
        coap_set_header_uri_path(request, "sensors-discovery");

        char query[32];
        snprintf(query, sizeof(query), "type=%s", sensor_types[i]);
        coap_set_header_uri_query(request, query);

        printf("[ACTUATOR LIGHT] Discovering sensor type: %s\n", sensor_types[i]);
        COAP_BLOCKING_REQUEST(&server_ep, request, discovery_response_handler);

        etimer_set(&reg_timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_UNTIL(etimer_expired(&reg_timer));
      }
    }
  }

  etimer_set(&et, OBSERVE_INTERVAL * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      for(int i = 0; i < 3; i++) {
          toggle_observation(i);
      }
    }

    if(ev == PROCESS_EVENT_TIMER && data == &led_timer) {
      if(energy_saving) {
        if(es_led_state) {
          leds_off(LEDS_BLUE);
          leds_off(LEDS_YELLOW);
        } else {
          leds_on(LEDS_BLUE);
          leds_on(LEDS_YELLOW);
        }
        es_led_state = !es_led_state;
        etimer_set(&led_timer, CLOCK_SECOND); 
      } else {
        leds_off(LEDS_YELLOW);
        leds_off(LEDS_BLUE);
        es_led_state = false;
      }
    }

    if (ev == button_hal_release_event) {
      printf("[ACTUATOR LIGHT] Button pressed\n");
      
      energy_saving = !energy_saving;
      if(energy_saving) {
        etimer_restart(&led_timer); // start blinking
        printf("[ACTUATOR LIGHT] Energy saving mode: ON\n");
      } else {
        leds_off(LEDS_YELLOW);
        leds_off(LEDS_BLUE);
        printf("[ACTUATOR LIGHT] Energy saving mode: OFF\n");
      }
    }

    if(ev == serial_line_event_message) {
      char *line = (char *)data;
      int new_min;

      if(strncmp(line, "light_th", 8) == 0) {
        if(sscanf(line + 8, "%d", &new_min) == 2) {
          if(new_min < 1000000) {
            min_light = new_min;
            printf("[ACTUATOR LIGHT] Updated thresholds: min=%.3f lux\n", min_light / 1000);
          } else {
            printf("[ACTUATOR LIGHT] Error: min must be < 1000 lux\n");
          }
        } else {
          printf("[ACTUATOR LIGHT] Format error. Use: light_th <min>\n");
        }
      } else {
        printf("[ACTUATOR LIGHT] Unknown command. Use: light_th <min>\n");
      }
    }
  }

  PROCESS_END();
}
