#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>
#include "dev/serial-line.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"

#include "../ml-classifier/room_occupancy_forecast.h"

#define OCCUPIED  0
#define NOT_OCCUPIED  1

#define LOG_MODULE "TempActuator"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_EP "coap://[fd00::1]/registration"
#define DISCOVERY_EP "coap://[fd00::1]/sensors-discovery"

static char* service_name = "actuator_temp";

static coap_endpoint_t server_ep;
static coap_message_t request[1];

static float min_temp = 18.0;
static float max_temp = 25.0;

static float room_data[3] = {0, 0, 0}; // 0=temp, 1=light, 2=humidity
static float room_occupancy_probability[2] = {0, 0};
static bool room_status;

static bool is_registered = false;
static char payload[128];
static bool sensors_discovered = false;

static char sensor_urls[3][128];  // parsed IPs + resource paths
static const char *sensor_types[] = { "temp", "light", "humidity" };
static const char *resource_paths[] = {
  "sensors/temp",
  "sensors/light",
  "sensors/humidity"
};

static int current_sensor_index = 0;

static bool is_connected() {
  if(NETSTACK_ROUTING.node_is_reachable()) {
    printf("[ACTUATOR TEMP] Connected to Border Router\n");
    return true;
  } else {
    printf("[ACTUATOR TEMP] Waiting for connection with the Border Router\n");
  }
  return false;
}

static void registration_response_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR TEMP] Registration request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(strncmp((char*)chunk, "Registered", len) == 0)
    is_registered = true;
  else
    is_registered = false;

  printf("[ACTUATOR TEMP] Registration response: %.*s\n", len, (char *)chunk);
}

static bool sensors_found[3] = {false, false, false};  // temp, light, humidity

static void discovery_parser(const uint8_t *chunk, int len) {
  char buffer[256];  
  int copy_len = len < (int)(sizeof(buffer) - 1) ? len : (int)(sizeof(buffer) - 1);

  memcpy(buffer, chunk, copy_len);
  buffer[copy_len] = '\0';

  printf("[ACTUATOR TEMP] Received discovery payload: %s\n", buffer);

  // Il messaggio ora contiene un solo sensore singolo, ma può finire con ';' o no
  // Rimuoviamo eventuale ';' finale
  if(buffer[copy_len - 1] == ';') {
    buffer[copy_len - 1] = '\0';
  }

  char type[32] = {0};
  char ip[96] = {0};

  if(sscanf(buffer, "sensor_%31[^@]@%95[^-]", type, ip) == 2) {
    printf("[ACTUATOR TEMP] Parsed sensor type='%s' ip='%s'\n", type, ip);

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
      printf("[ACTUATOR TEMP] Unknown sensor type: %s\n", type);
    }
  } else {
    printf("[ACTUATOR TEMP] Parsing failed for discovery payload\n");
  }

  if(sensors_found[0] && sensors_found[1] && sensors_found[2]) {
    sensors_discovered = true;
    printf("[ACTUATOR TEMP] All sensors discovered:\n  Temp: %s\n  Light: %s\n  Humidity: %s\n",
           sensor_urls[0], sensor_urls[1], sensor_urls[2]);
  } else {
    printf("[ACTUATOR TEMP] Waiting for more sensors to be discovered...\n");
  }
}

static void discovery_response_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR TEMP] Sensor discovery timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(len <= 0) {
    printf("[ACTUATOR TEMP] Sensor discovery failed: empty payload\n");
    return;
  }

  discovery_parser(chunk, len);
}

static void client_chunk_handler(coap_message_t *response) {
  if (response == NULL) {
    printf("[ACTUATOR TEMP] Timeout from sensor\n");
    return;
  }

  const uint8_t *payload;
  int len = coap_get_payload(response, &payload);

  if (len > 0) {
    char value_str[16];
    memcpy(value_str, payload, len);
    value_str[len] = '\0';
    float value = atof(value_str) / 1000;

    room_data[current_sensor_index] = value;

    if(current_sensor_index == 0) {
      printf("[ACTUATOR TEMP] Temperature: %.3f C\n", value);

      leds_off(LEDS_RED);
      leds_off(LEDS_YELLOW);
      if(value < min_temp) {
        leds_on(LEDS_RED);
        leds_off(LEDS_YELLOW);
        printf("[ACTUATOR TEMP] Temperature LOW - Heater ON (RED LED)\n");
      } else if(value > max_temp) {
        leds_off(LEDS_RED);
        leds_on(LEDS_YELLOW);
        printf("[ACTUATOR TEMP] Temperature HIGH - Cooler ON (YELLOW LED)\n");
      } else {
        leds_off(LEDS_RED);
        leds_off(LEDS_YELLOW);
        printf("[ACTUATOR TEMP] Temperature in range - Device OFF \n");
      }

    } else if(current_sensor_index == 1) {
      printf("[ACTUATOR TEMP] Light: %.3f lux\n", value);
    } else if(current_sensor_index == 2) {
      printf("[ACTUATOR TEMP] Humidity: %.3f %%\n", value);

      printf("[ACTUATOR TEMP] Room data: %.3f C; %.3f lux; %.3f%%;\n", room_data[0], room_data[1], room_data[2]);
      eml_net_predict_proba(&room_occupancy_forecast, room_data, 3, room_occupancy_probability, 2);
      if (room_occupancy_probability[0] > room_occupancy_probability[1]){
        room_status = OCCUPIED;
        printf("[ACTUATOR TEMP] Not occupied: %.3f%% - Occupied: %.3f%% - Status: Not occupied\n", room_occupancy_probability[0]*100, room_occupancy_probability[1]*100);
      } else {
        room_status = NOT_OCCUPIED;
        printf("[ACTUATOR TEMP] Not occupied: %.3f%% - Occupied: %.3f%% - Status: Occupied\n", room_occupancy_probability[0]*100, room_occupancy_probability[1]*100);
      }
    }
  }
}

PROCESS(temp_actuator, "Temp Actuator");
AUTOSTART_PROCESSES(&temp_actuator);

PROCESS_THREAD(temp_actuator, ev, data)
{
  static struct etimer reg_timer;
  static struct etimer connectivity_timer;
  static struct etimer et;

  PROCESS_BEGIN();

  printf("[ACTUATOR TEMP] Starting CoAP Temp Actuator\n");
  printf("[ACTUATOR TEMP] Temperature thresholds: min=%.3f lux max=%.3f lux\n", min_temp, max_temp);
  printf("[ACTUATOR TEMP] Dummy call to avoid warnings: %p, %s\n", eml_net_activation_function_strs, eml_error_str(0));

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

    printf("[ACTUATOR TEMP] Sending registration for %s\n", service_name);
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

        printf("[ACTUATOR TEMP] Discovering sensor type: %s\n", sensor_types[i]);
        COAP_BLOCKING_REQUEST(&server_ep, request, discovery_response_handler);

        etimer_set(&reg_timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_UNTIL(etimer_expired(&reg_timer));
      }
    }
  }

  etimer_set(&et, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&et)) {
      for(current_sensor_index = 0; current_sensor_index < 3; current_sensor_index++) {
        coap_endpoint_parse(sensor_urls[current_sensor_index],
                            strlen(sensor_urls[current_sensor_index]),
                            &server_ep);

        coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
        coap_set_header_uri_path(request, resource_paths[current_sensor_index]);

        printf("[ACTUATOR TEMP] Sending GET to %s (%s)\n",
               sensor_urls[current_sensor_index],
               resource_paths[current_sensor_index]);

        COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
      }

      etimer_reset(&et);
    }

    if(ev == serial_line_event_message) {
      char *line = (char *)data;
      int new_min, new_max;

      if(strncmp(line, "temp_th", 8) == 0) {
        if(sscanf(line + 8, "%d %d", &new_min, &new_max) == 2) {
          if(new_min < new_max) {
            min_temp = new_min;
            max_temp = new_max;
            printf("[ACTUATOR TEMP] Updated thresholds: min=%.3f lux max=%.3f lux\n",
                   min_temp, max_temp);
          } else {
            printf("[ACTUATOR TEMP] Error: min must be < max\n");
          }
        } else {
          printf("[ACTUATOR TEMP] Format error. Use: temp_th <min> <max>\n");
        }
      } else {
        printf("[ACTUATOR TEMP] Unknown command. Use: temp_th <min> <max>\n");
      }
    }
  }

  PROCESS_END();
}
