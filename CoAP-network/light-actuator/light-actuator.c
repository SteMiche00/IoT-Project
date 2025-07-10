#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dev/serial-line.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"

#include "../ml-classifier/room_occupancy_forecast.h"

#define OCCUPIED       0
#define NOT_OCCUPIED   1

#define LOG_MODULE "LightActuator"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_EP     "coap://[fd00::1]/registration"
#define DISCOVERY_EP  "coap://[fd00::1]/sensors-discovery"

static char *service_name = "actuator_light";

static void notify_handler_light(coap_message_t *msg);
static void notify_handler_temp(coap_message_t *msg);
static void notify_handler_humidity(coap_message_t *msg);

static coap_endpoint_t br_ep;          
static coap_message_t request[1];      

static float min_light = 300.0f;
static float max_light = 800.0f;

static float room_data[3] = { -99, -99, -99 }; // 0:Temperature,1:Light,2:Humidity
static float room_occupancy_probability[2];
static bool  room_status;

static bool is_registered   = false;
static bool sensors_discovered = false;
static bool sensors_found[3] = { false, false, false }; // 0:Temperature,1:Light,2:Humidity

static char payload[64];
static char sensor_urls[3][128];       
static const char *sensor_types[3] = { "temp", "light", "humidity" };
static const char *resource_paths[3] = { "sensors/temp", "sensors/light", "sensors/humidity" };

/* One 2‑byte token per sensor so we can route notifications. */
static uint8_t observe_tokens[3][2] = { {0xaa,0x01}, {0xaa,0x02}, {0xaa,0x03} };

/*---------------------------------------------------------------------------*/
static bool
is_connected(void)
{
  if(NETSTACK_ROUTING.node_is_reachable()) {
    printf("[ACTUATOR] Connected to Border Router\n");
    return true;
  }
  printf("[ACTUATOR] Waiting for connection with the Border Router\n");
  return false;
}

/*---------------------------------------------------------------------------*/
static void
registration_handler(coap_message_t *response)
{
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR] Registration timed out\n");
    return;
  }
  int len = coap_get_payload(response, &chunk);
  is_registered = (len >= 9 && strncmp((const char *)chunk, "Registered", 9) == 0);
  printf("[ACTUATOR] Registration response: %.*s\n", len, (char *)chunk);
}

/*---------------------------------------------------------------------------*/
static void discovery_parser(const uint8_t *chunk, int len);

static void
discovery_handler(coap_message_t *response)
{
  const uint8_t *chunk;
  if(response == NULL) {
    printf("[ACTUATOR] Discovery timeout\n");
    return;
  }
  int len = coap_get_payload(response, &chunk);
  if(len > 0) {
    discovery_parser(chunk, len);
  }
}

static void
start_observe_for_sensor(int idx)
{
  coap_endpoint_t sensor_ep;
  coap_endpoint_parse(sensor_urls[idx], strlen(sensor_urls[idx]), &sensor_ep);
  coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
  coap_set_header_uri_path(request, resource_paths[idx]);
  coap_set_header_observe(request, 0);
  coap_set_token(request, observe_tokens[idx], sizeof(observe_tokens[idx]));
  /* We do NOT use the blocking variant so that the process can keep
     running and receive asynchronous notifications. */
  coap_send_request(&sensor_ep, request);
  printf("[ACTUATOR] Sent OBSERVE to %s (%s)\n", sensor_urls[idx], resource_paths[idx]);
}

static void
discovery_parser(const uint8_t *chunk, int len)
{
  char buf[256];
  len = len < 255 ? len : 255;
  memcpy(buf, chunk, len);
  buf[len] = '\0';

  char type[32]={0};
  char ip[96]={0};
  if(sscanf(buf, "sensor_%31[^@]@%95[^-]", type, ip) == 2) {
    int idx = -1;
    if(!strcmp(type,"temp")) idx = 0;
    else if(!strcmp(type,"light"))idx = 1;
    else if(!strcmp(type,"humidity"))idx = 2;

    if(idx >= 0 && !sensors_found[idx]) {
      snprintf(sensor_urls[idx], sizeof(sensor_urls[idx]), "coap://[%s]", ip);
      sensors_found[idx] = true;
      printf("[ACTUATOR] Discovered %s sensor at %s\n", type, sensor_urls[idx]);
      /* Once discovered we immediately start observing. */
      start_observe_for_sensor(idx);
    }
  }
  sensors_discovered = sensors_found[0] && sensors_found[1] && sensors_found[2];
}

/*---------------------------------------------------------------------------*/
static void
notify_handler_light(coap_message_t *msg)
{
  const uint8_t *payload; int len = coap_get_payload(msg,&payload);
  if(len<=0) return;
  char v[16]; memcpy(v,payload,len); v[len]='\0';
  float value = atof(v)/1000.0f;
  room_data[1] = value;
  printf("[OBS] Light: %.3f lux\n", value);
  leds_off(LEDS_YELLOW);
  if(value < min_light) {
    leds_on(LEDS_YELLOW);
    printf("[OBS] Light LOW → LED ON\n");
  }
}

static void
notify_handler_temp(coap_message_t *msg)
{
  const uint8_t *payload; int len = coap_get_payload(msg,&payload);
  if(len<=0) return;
  char v[16]; memcpy(v,payload,len); v[len]='\0';
  room_data[0] = atof(v)/1000.0f;
  printf("[OBS] Temp: %.3f °C\n", room_data[0]);
}

static void
notify_handler_humidity(coap_message_t *msg)
{
  const uint8_t *payload; int len = coap_get_payload(msg,&payload);
  if(len<=0) return;
  char v[16]; memcpy(v,payload,len); v[len]='\0';
  room_data[2] = atof(v)/1000.0f;
  printf("[OBS] Humidity: %.3f %%\n", room_data[2]);
}

/*---------------------------------------------------------------------------*/
static void
handle_incoming_notification(void)
{
  coap_message_t *msg = coap_get_last_received();
  if(msg == NULL) return;

  for(int i=0;i<3;i++) {
    if(msg->token_len == 2 && memcmp(msg->token, observe_tokens[i], 2)==0) {
      switch(i) {
        case 0: notify_handler_temp(msg); break;
        case 1: notify_handler_light(msg); break;
        case 2: notify_handler_humidity(msg); break;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
PROCESS(light_actuator, "Light Actuator");
AUTOSTART_PROCESSES(&light_actuator);

PROCESS_THREAD(light_actuator, ev, data)
{
  static struct etimer periodic;
  static struct etimer retry;
  PROCESS_BEGIN();

  printf("[ACTUATOR] Starting CoAP Light Actuator (Observe‑friendly)\n");
  printf("[ACTUATOR] Thresholds: %.1f – %.1f lux\n", min_light, max_light);
  printf("[ACTUATOR] Dummy ML link: %p, %s\n", eml_net_activation_function_strs, eml_error_str(0));

  /* Wait until the RPL DAG is up. */
  while(!is_connected()) {
    etimer_set(&retry, CLOCK_SECOND*5);
    PROCESS_WAIT_UNTIL(etimer_expired(&retry));
  }

  /* Register at the BR. */
  while(!is_registered) {
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &br_ep);
    coap_init_message(request, COAP_TYPE_CON, COAP_POST, coap_get_mid());
    coap_set_header_uri_path(request, "registration");
    snprintf(payload,sizeof(payload),"%s",service_name);
    coap_set_payload(request,(uint8_t*)payload,strlen(payload));
    COAP_BLOCKING_REQUEST(&br_ep, request, registration_handler);
    etimer_set(&retry, CLOCK_SECOND*5);
    PROCESS_WAIT_UNTIL(etimer_expired(&retry));
  }

  /* Discover sensors (loop until all found). */
  while(!sensors_discovered) {
    for(int i=0;i<3;i++) if(!sensors_found[i]) {
      coap_endpoint_parse(DISCOVERY_EP, strlen(DISCOVERY_EP), &br_ep);
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
      coap_set_header_uri_path(request, "sensors-discovery");
      char q[32]; snprintf(q,sizeof(q),"type=%s",sensor_types[i]);
      coap_set_header_uri_query(request,q);
      COAP_BLOCKING_REQUEST(&br_ep, request, discovery_handler);
      etimer_set(&retry, CLOCK_SECOND*2);
      PROCESS_WAIT_UNTIL(etimer_expired(&retry));
    }
  }

  etimer_set(&periodic, CLOCK_SECOND*30);

  /* ---------------------  MAIN LOOP  --------------------- */
  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_POLL) {
      handle_incoming_notification();
    }

    /* Periodic polling fallback for missed samples & feeding the ML model. */
    if(ev == PROCESS_EVENT_TIMER && data == &periodic) {
      for(int idx=0; idx<3; idx++) {
        coap_endpoint_t sensor_ep;
        coap_endpoint_parse(sensor_urls[idx], strlen(sensor_urls[idx]), &sensor_ep);
        coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
        coap_set_header_uri_path(request, resource_paths[idx]);
        COAP_BLOCKING_REQUEST(&sensor_ep, request, /*reuse*/notify_handler_light);
      }
      /* After we have T,L,H we can run the ML model. */
      if(room_data[0]!=-99 && room_data[1]!=-99 && room_data[2]!=-99) {
        eml_net_predict_proba(&room_occupancy_forecast, room_data, 3,
                              room_occupancy_probability, 2);
        room_status = (room_occupancy_probability[1] > room_occupancy_probability[0]) ? NOT_OCCUPIED : OCCUPIED;
        printf("[ACTUATOR] ML → OCC %s (%.1f%%)\n",
               room_status==OCCUPIED?"NO":"YES",
               100*room_occupancy_probability[1]);
      }
      etimer_reset(&periodic);
    }

    /* Serial command for thresholds. */
    if(ev == serial_line_event_message) {
      char *line = (char *)data; float a,b;
      if(strncmp(line,"light_th",8)==0 && sscanf(line+8,"%f %f",&a,&b)==2 && a<b) {
        min_light=a; max_light=b;
        printf("[ACTUATOR] New thresholds: %.1f–%.1f lux\n",min_light,max_light);
      } else {
        printf("[ACTUATOR] Usage: light_th <min> <max>\n");
      }
    }
  }

  PROCESS_END();
}
