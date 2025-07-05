#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include <stdio.h>

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_light,
         "title=\"Light\";rt=\"LightSensor\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int light = rand() % 1000001; //  [0; 1.000.000] millilux
  int len = snprintf((char *)buffer, preferred_size, "%d", light);

  printf("[SENSOR LIGHT] Received request on /sensors/light - value: %.3f lux\n", light / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

PROCESS(coap_light_sensor_process, "CoAP Light Sensor");
AUTOSTART_PROCESSES(&coap_light_sensor_process);

PROCESS_THREAD(coap_light_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting CoAP Light Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_light, "sensors/light");

  PROCESS_END();
}
