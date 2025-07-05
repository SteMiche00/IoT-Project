#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include <stdio.h>

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
  int humidity = rand() % 100001; // [0; 100000] --> from 0% to 100% humidity value
  int len = snprintf((char *)buffer, preferred_size, "%d", humidity);

  printf("[SENSOR HUMIDITY] Received request on /sensors/humidity - value: %.3f \n", humidity / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

PROCESS(coap_humidity_sensor_process, "CoAP Humidity Sensor");
AUTOSTART_PROCESSES(&coap_humidity_sensor_process);

PROCESS_THREAD(coap_humidity_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting CoAP Humidity Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_humidity, "sensors/humidity");

  PROCESS_END();
}
