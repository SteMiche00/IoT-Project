#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/uip-ds6.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-debug.h"
#include "sys/log.h"

#define LOG_MODULE "BorderRouter"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(border_router_process, "Border Router");
AUTOSTART_PROCESSES(&border_router_process);

/* Funzione per stampare indirizzi IPv6 */
static void print_local_addresses(void) {
  int i;
  LOG_INFO("Local IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if(uip_ds6_if.addr_list[i].isused) {
      LOG_INFO_6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      LOG_INFO_("\n");
    }
  }
}

PROCESS_THREAD(border_router_process, ev, data) {
  PROCESS_BEGIN();

  LOG_INFO("Starting Border Router\n");

  /* Attende che la rete sia pronta e avvia RPL */
  NETSTACK_ROUTING.root_start();

  /* Mostra indirizzi IPv6 assegnati */
  print_local_addresses();

  PROCESS_END();
}
