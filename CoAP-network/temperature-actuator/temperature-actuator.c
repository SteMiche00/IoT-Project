#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>
#include "dev/serial-line.h"

#define SERVER_EP "coap://[fe80::202:2:2:2]"  // Cambia con l'indirizzo del sensore

static struct etimer et;
static coap_endpoint_t server_ep;
static coap_message_t request[1];
static int min_temperature = 20;
static int max_temperature = 28;

PROCESS(led_actuator_client, "LED Actuator CoAP Client");
AUTOSTART_PROCESSES(&led_actuator_client);

// Funzione di callback per gestire la risposta dal sensore
static void client_chunk_handler(coap_message_t *response) {
  if (response == NULL) {
    printf("[ACTUATOR] Timeout nella risposta\n");
    return;
  }

  const uint8_t *payload;
  int len = coap_get_payload(response, &payload);
  if (len > 0) {
    char temp_str[8];
    memcpy(temp_str, payload, len);
    temp_str[len] = '\0';
    int temp = atoi(temp_str);
    printf("[ACTUATOR] Temperatura ricevuta: %d\n", temp);

    // Accendi led in base alla temperatura
    leds_off(LEDS_RED | LEDS_YELLOW);  // spegni entrambi prima

    if (temp < min_temperature) {
      leds_on(LEDS_RED);   // temperatura bassa -> scaldare -> rosso acceso
      printf("[ACTUATOR] Temperatura bassa: accendo LED rosso\n");
    } else if (temp > max_temperature) {
      leds_on(LEDS_YELLOW); // temperatura alta -> raffreddare -> giallo acceso
      printf("[ACTUATOR] Temperatura alta: accendo LED giallo\n");
    } else {
      printf("[ACTUATOR] Temperatura normale: LED spenti\n");
    }
  }
}

PROCESS_THREAD(led_actuator_client, ev, data) {
  PROCESS_BEGIN();

  coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

  etimer_set(&et, CLOCK_SECOND * 10);

  printf("[ACTUATOR] Soglie iniziali: min=%d max=%d\n", min_temperature, max_temperature);
  printf("[ACTUATOR] Per cambiare soglie, scrivi 'min max' seguito da invio (es: 18 26)\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == serial_line_event_message) {
      char *line = (char *)data;
      int new_min, new_max;
      if(sscanf(line, "%d %d", &new_min, &new_max) == 2) {
        if(new_min < new_max) {
          min_temperature = new_min;
          max_temperature = new_max;
          printf("[ACTUATOR] Soglie aggiornate: min=%d max=%d\n", min_temperature, max_temperature);
        } else {
          printf("[ACTUATOR] Errore: min deve essere minore di max\n");
        }
      } else {
        printf("[ACTUATOR] Formato input errato. Usa: min max\n");
      }
    }

    if(etimer_expired(&et)) {
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, coap_get_mid());
      coap_set_header_uri_path(request, "sensors/temp");

      printf("[ACTUATOR] Invio GET a %s/sensors/temp\n", SERVER_EP);

      COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

      etimer_reset(&et);
    }
  }

  PROCESS_END();
}

