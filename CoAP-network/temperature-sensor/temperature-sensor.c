#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include <stdio.h>

/* Risorsa simulata: temperatura */
static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_temp,
         "title=\"Temperature\";rt=\"TemperatureSensor\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int temp = 20 + (rand() % 10); // Temperatura simulata
  int len = snprintf((char *)buffer, preferred_size, "%d", temp);

  printf("[SENSOR] Richiesta ricevuta su /sensors/temp - invio valore: %d C\n", temp);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

PROCESS(coap_sensor_process, "CoAP Temperature Sensor");
AUTOSTART_PROCESSES(&coap_sensor_process);

PROCESS_THREAD(coap_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting CoAP Temperature Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_temp, "sensors/temp");

  PROCESS_END();
}
