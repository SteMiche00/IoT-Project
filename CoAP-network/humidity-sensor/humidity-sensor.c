#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include <stdio.h>
#include "coap-blocking-api.h"
#include "sys/log.h"

#define LOG_MODULE "HumiditySensor"
#define LOG_LEVEL LOG_LEVEL_INFO
#define SERVER_EP "coap://[fd00::1]/register"

static struct etimer et;
static char* service_name = "sensor_humidity";
static char* service_type = "humidity";
static char* service_resource = "/sensors/humidity";

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_humidity,
         "title=\"Humidity\";rt=\"HumiditySensor\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int humidity = rand() % 100001; // [0; 100000] --> from 0% to 100%
  int len = snprintf((char *)buffer, preferred_size, "%d", humidity);

  printf("[SENSOR HUMIDITY] Received request on /sensors/humidity - value: %.3f \n", humidity / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[SENSOR HUMIDITY] Request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  printf("[SENSOR HUMIDITY] Response: %.*s\n", len, (char *)chunk);
}

PROCESS(coap_humidity_sensor_process, "CoAP Humidity Sensor");
AUTOSTART_PROCESSES(&coap_humidity_sensor_process);

PROCESS_THREAD(coap_humidity_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting CoAP Humidity Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_humidity, "sensors/humidity");

  etimer_set(&et, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  static coap_endpoint_t server_ep;
  static coap_message_t request[1];

  coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
  coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
  coap_set_header_uri_path(request, "register");

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"name\":\"%s\",\"ip\":\"%s\",\"type\":\"%s\",\"resource\":\"%s\"}",
           service_name, "fd00::abcd", service_type, service_resource);

  coap_set_payload(request, (uint8_t *)payload, strlen(payload));

  printf("[SENSOR HUMIDITY] Sending registration for %s\n", service_name);

  COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

  PROCESS_END();
}
