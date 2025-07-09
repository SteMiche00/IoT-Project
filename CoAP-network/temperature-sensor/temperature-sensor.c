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

#define LOG_MODULE "TempSensor"
#define LOG_LEVEL LOG_LEVEL_INFO
#define SERVER_EP "coap://[fd00::1]/registration"

static char* service_name = "sensor_temp";
static struct etimer reg_timer;
static struct etimer connectivity_timer;
static int last_temp_value = 0;
static bool is_registered = false;
static coap_endpoint_t server_ep;
static coap_message_t request[1];
char payload[128];
static struct etimer sensor_timer;

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_temp,
         "title=\"Temperature\";rt=\"TemperatureSensor\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int len = snprintf((char *)buffer, preferred_size, "%d", last_temp_value);

  printf("[SENSOR TEMP] Received request on /sensors/temp - returning: %.3f C\n", last_temp_value / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static bool is_connected() {
  if(NETSTACK_ROUTING.node_is_reachable()) {
    printf("[SENSOR TEMP] Connected to Border Router\n");
    return true;
  } else {
    printf("[SENSOR TEMP] Waiting for connection with the Border Router\n");
  }
  return false;
}

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[SENSOR TEMP] Registration request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(strncmp((char*)chunk, "Registered", len) == 0)
    is_registered = true;
  else
    is_registered = false;

  printf("[SENSOR TEMP] Response: %.*s\n", len, (char *)chunk);
}

PROCESS(coap_temp_sensor_process, "CoAP Temperature Sensor");
AUTOSTART_PROCESSES(&coap_temp_sensor_process);

PROCESS_THREAD(coap_temp_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("[SENSOR TEMP] Starting CoAP Temperature Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_temp, "sensors/temp");

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

    printf("[SENSOR TEMP] Sending registration for %s\n", service_name);
    COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

    etimer_set(&reg_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reg_timer));
  }

  etimer_set(&sensor_timer, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sensor_timer));

    last_temp_value = (rand() % 40001) - 10000; 
    printf("[SENSOR TEMP] New temperature value generated: %.3f C\n", last_temp_value / 1000.0);

    etimer_reset(&sensor_timer);
  }

  PROCESS_END();
}
