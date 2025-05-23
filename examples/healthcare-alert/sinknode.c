//test change
#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234

static struct uip_udp_conn *udp_conn;

PROCESS(sink_node_process, "Healthcare Sink Node");
AUTOSTART_PROCESSES(&sink_node_process);

PROCESS_THREAD(sink_node_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Sink node started, listening on port %d\n", UDP_PORT);

  udp_conn = udp_new(NULL, UIP_HTONS(UDP_PORT), NULL);
  udp_bind(udp_conn, UIP_HTONS(UDP_PORT));

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event && uip_newdata()) {
      char *received_data = (char *)uip_appdata;
      int len = uip_datalen();
      char msg[len + 1];
      memcpy(msg, received_data, len);
      msg[len] = '\0';

      leds_on(LEDS_GREEN);
      printf("✅ Received alert: %s\n", msg);
    }
  }

  PROCESS_END();
}
