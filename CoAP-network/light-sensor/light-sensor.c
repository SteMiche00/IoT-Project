#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "coap-blocking-api.h"
#include "coap-observe.h"
#include "node-id.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"
#include "sys/log.h"

#define LOG_MODULE "LightSensor"
#define LOG_LEVEL LOG_LEVEL_INFO
#define SERVER_EP "coap://[fd00::1]/registration"

static char* service_name = "sensor_light";
static struct etimer reg_timer;
static struct etimer connectivity_timer;
static int last_light_value = 268443;
static bool is_registered = false;
static coap_endpoint_t server_ep;
static coap_message_t request[1];
char payload[128];
static struct etimer sensor_timer;
static struct etimer reg_led_timer;

static int direction = -1;        // 1 = increasing, -1 = decreasing

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_light,
         "title=\"Light\";rt=\"LightSensor\";obs",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int len = snprintf((char *)buffer, preferred_size, "%d", last_light_value);

  printf("[SENSOR LIGHT] Received request on /sensors/light - returning: %.3f lux\n", last_light_value / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static bool is_connected() {
	if(NETSTACK_ROUTING.node_is_reachable()) {
		printf("[SENSOR LIGHT] Connected to Border Router\n");
		return true;
  	} else {
		printf("[SENSOR LIGHT] Waiting for connection with the Border Router\n");
	}
	return false;
}

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[SENSOR LIGHT] Registration request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  if(strncmp((char*)chunk, "Registered", len) == 0)
    is_registered = true;
  else if (strncmp((char*)chunk, "Unregistered", len) == 0)
    is_registered = false;
  else if (strncmp((char*)chunk, "Failed", len) == 0)
    is_registered = false;

  printf("[SENSOR LIGHT] Response: %.*s\n", len, (char *)chunk);
}

PROCESS(coap_light_sensor_process, "CoAP Light Sensor");
AUTOSTART_PROCESSES(&coap_light_sensor_process);

PROCESS_THREAD(coap_light_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  printf("[SENSOR LIGHT] Starting CoAP Light Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_light, "sensors/light");
  res_light.flags |= IS_OBSERVABLE;

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

    printf("[SENSOR LIGHT] Sending registration for %s\n", service_name);
    COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

    etimer_set(&reg_timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reg_timer));
  }

  etimer_set(&sensor_timer, CLOCK_SECOND * 10);
  etimer_set(&reg_led_timer, CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER && data == &reg_led_timer) {
      etimer_reset(&reg_led_timer);

      if(is_registered) {
        leds_toggle(LEDS_GREEN);  
      } else {
        leds_off(LEDS_GREEN);      
      }
    }

    if(ev == PROCESS_EVENT_TIMER && data == &sensor_timer){

      // last_light_value = rand() % 900001; 
      last_light_value += direction * 23391; 

      if(last_light_value >= 500000) {
        last_light_value = 500000;
        direction = -1;
      } else if(last_light_value <= 100000) {
        last_light_value = 100000;
        direction = 1;
      }
      printf("[SENSOR LIGHT] New light value generated: %.3f lux\n", last_light_value / 1000.0);
      coap_notify_observers(&res_light);
      etimer_reset(&sensor_timer);
    }
  }

  PROCESS_END();
}
