
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


#define LOG_MODULE "LightSensor"
#define LOG_LEVEL LOG_LEVEL_INFO
//#define SERVER_EP "coap://[fd00::bf25:8c77:1f3e:6e1]/registration"
#define SERVER_EP "coap://[fd00::1]/registration"


static char* service_name = "sensor_light";
// static char* service_type = "light";
// static char* service_resource = "/sensors/light";


static struct etimer reg_timer;
static struct etimer connectivity_timer;
static int last_light_value = 0;
static bool registered = false;

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
  int len = snprintf((char *)buffer, preferred_size, "%d", last_light_value);

  printf("[SENSOR LIGHT] Received request on /sensors/light - returning: %.3f lux\n", last_light_value / 1000.0);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);
}

static bool is_connected() {
	if(NETSTACK_ROUTING.node_is_reachable()) {
		LOG_INFO("The Border Router is reachable\n");
		return true;
  	} else {
		LOG_INFO("Waiting for connection with the Border Router\n");
	}
	return false;
}

void client_chunk_handler(coap_message_t *response) {
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[SENSOR LIGHT] Request timed out\n");
    return;
  }

  int len = coap_get_payload(response, &chunk);
  printf("[SENSOR LIGHT] Response: %.*s\n", len, (char *)chunk);
}

/*
static void get_global_ipaddr(char *ip_str, size_t len) {
  int i;
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if(uip_ds6_if.addr_list[i].isused &&
       uip_ds6_if.addr_list[i].state == ADDR_PREFERRED) {
      uiplib_ipaddr_snprint(ip_str, len, &uip_ds6_if.addr_list[i].ipaddr);
      return;
    }
  }
  snprintf(ip_str, len, "IP_NOT_FOUND");
}
*/

PROCESS(coap_light_sensor_process, "CoAP Light Sensor");
AUTOSTART_PROCESSES(&coap_light_sensor_process);

PROCESS_THREAD(coap_light_sensor_process, ev, data)
{
  PROCESS_BEGIN();

  static coap_endpoint_t server_ep;
  static coap_message_t request[1];
  char payload[128];
  printf("Starting CoAP Light Sensor\n");

  coap_engine_init();
  coap_activate_resource(&res_light, "sensors/light");

  /*
   // --- Ping Border Router
  if(uiplib_ipaddrconv("fd00::1", &dest_ip)) {
    printf("[SENSOR LIGHT] Sending ping to fd00::1\n");
    uip_icmp6_send(&dest_ip, ICMP6_ECHO_REQUEST, 0, 4);
  }
*/
  while(!is_connected()){
    etimer_reset(&connectivity_timer);
		PROCESS_WAIT_UNTIL(etimer_expired(&connectivity_timer));
  }

  // --- Retry loop: register to CoAP server
  while(!registered) {
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
  static struct etimer sensor_timer;
  etimer_set(&sensor_timer, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sensor_timer));

    last_light_value = rand() % 1000001; // millilux
    printf("[SENSOR LIGHT] New light value generated: %.3f lux\n", last_light_value / 1000.0);

    etimer_reset(&sensor_timer);
  }

  PROCESS_END();
}
