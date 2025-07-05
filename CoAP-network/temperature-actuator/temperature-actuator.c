#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>
#include "dev/serial-line.h"

#include "../ml-classifier/room_occupancy_forecast.h"

#define SENSOR_TEMPERATURE "coap://[fe80::202:2:2:2]"
#define SENSOR_LIGHT       "coap://[fe80::203:3:3:3]"
#define SENSOR_HUMIDITY    "coap://[fe80::204:4:4:4]"
#define OCCUPIED  0
#define NOT_OCCUPIED  1

static struct etimer et;
static coap_endpoint_t server_ep;
static coap_message_t request[1];

static float min_temperature = 20.000;
static float max_temperature = 28.000;

static float room_data[3] = {0, 0, 0}; // 0=temp, 1=light, 2=humidity
static float room_occupancy_probability[2] = {0, 0};
static bool room_status;

PROCESS(temperature_actuator, "Temperature Actuator");
AUTOSTART_PROCESSES(&temperature_actuator);

static int current_sensor_index = 0;
static const char *sensor_urls[] = {
  SENSOR_TEMPERATURE,
  SENSOR_LIGHT,
  SENSOR_HUMIDITY
};

static const char *resource_paths[] = {
  "sensors/temp",
  "sensors/light",
  "sensors/humidity"
};

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

    if(current_sensor_index == 0) { // Temperature
      printf("[ACTUATOR TEMP] Temperature: %.3f C\n", value);
      leds_off(LEDS_RED | LEDS_YELLOW);
      if (value < min_temperature) {
        leds_on(LEDS_RED);
        printf("[ACTUATOR TEMP] Temperature LOW - Heating ON (RED LED)\n");
      } else if (value > max_temperature) {
        leds_on(LEDS_YELLOW);
        printf("[ACTUATOR TEMP] Temperature HIGH - Cooling ON (YELLOW LED)\n");
      } else {
        printf("[ACTUATOR TEMP] Temperature NORMAL - LEDs OFF\n");
      }
    } else if(current_sensor_index == 1) { // Light
      printf("[ACTUATOR TEMP] Light: %.3f lux\n", value);
    } else if(current_sensor_index == 2) { // Humidity
      printf("[ACTUATOR TEMP] Humidity: %.3f %%\n", value);

      // Inference
      printf("[ACTUATOR TEMP] Room data - %.3f C - %.3f lux - %.3f%%\n", room_data[0], room_data[1], room_data[2]);
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

PROCESS_THREAD(temperature_actuator, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND * 10);
  printf("Starting CoAP Temperature Actuator\n");
  printf("[ACTUATOR TEMP] Thresholds: min=%.3f C max=%.3f C\n", min_temperature, max_temperature);
  printf("[ACTUATOR TEMP] Dummy call to avoid errors: %p, %s\n", eml_net_activation_function_strs, eml_error_str(0)); 

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

      if(strncmp(line, "temp_th", 7) == 0) {
        if(sscanf(line + 7, "%d %d", &new_min, &new_max) == 2) {
          if(new_min < new_max) {
            min_temperature = new_min;
            max_temperature = new_max;
            printf("[ACTUATOR TEMP] Updated thresholds: min=%.3f C max=%.3f C\n",
                  min_temperature, max_temperature);
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
