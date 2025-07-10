#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "coap-blocking-api.h"
#include "node-id.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"
#include "sys/log.h"

#define LOG_MODULE "HumSensor"
#define LOG_LEVEL LOG_LEVEL_INFO
#define SERVER_EP "coap://[fd00::1]/registration"

static char* service_name = "sensor_humidity";
static struct etimer reg_timer;
static struct etimer connectivity_timer;
static int last_humidity_value = 0;
static bool is_registered = false;
static coap_endpoint_t server_ep;
static coap_message_t request[1];
char payload[128];
static struct etimer sensor_timer;

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_humidity,
         "title=\"Humidity\";rt=\"HumiditySensor\";obs",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int len = snprintf((char *)buffer, preferred_size, "%d", last_humidity_value);

  printf("[SENSOR HUMIDITY] Received request on /sensors/humidity - returning: %.3f %%\n", last_humidity_value / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static bool is_connected() {
  if(NETSTACK_ROUTING.node_is_reachable()) {
    printf("[SENSOR HUMIDITY] Connected to Border Router\n");
    return true;
  } else {
    printf("[SENSOR HUMIDITY] Waiting for connection with the Border Router\n");
  }
  return false;
}

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[SENSOR HUMIDITY] Registration request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(strncmp((char*)chunk, "Registered", len) == 0)
    is_registered = true;
  else if (strncmp((char*)chunk, "Unregistered", len) == 0)
    is_registered = false;
  else if (strncmp((char*)chunk, "Failed", len) == 0)
    is_registered = false;

  printf("[SENSOR HUMIDITY] Response: %.*s\n", len, (char *)chunk);
}

PROCESS(coap_humidity_sensor_process, "CoAP Humidity Sensor");
AUTOSTART_PROCESSES(&coap_humidity_sensor_process);

PROCESS_THREAD(coap_humidity_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("[SENSOR HUMIDITY] Starting CoAP Humidity Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_humidity, "sensors/humidity");
  res_humidity.flags |= IS_OBSERVABLE;

  while(!is_connected()){
    etimer_set(&connectivity_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_UNTIL(etimer_expired(&connectivity_timer));
  }

  while(!is_registered) {
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
    coap_set_header_uri_path(request, "registration");
    snprintf(payload, sizeof(payload), "%s", service_name);
    coap_set_payload(request, (uint8_t *)payload, strlen(payload));

    printf("[SENSOR HUMIDITY] Sending registration for %s\n", service_name);
    COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

    etimer_set(&reg_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reg_timer));
  }

  etimer_set(&sensor_timer, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sensor_timer));

    last_humidity_value = rand() % 100000; 
    printf("[SENSOR HUMIDITY] New humidity value generated: %.3f %%\n", last_humidity_value / 1000.0);

    coap_notify_observers(&res_humidity);

    etimer_reset(&sensor_timer);
  }

  PROCESS_END();
}
