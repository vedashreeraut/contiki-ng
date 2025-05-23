#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234

static struct uip_udp_conn *udp_conn;
static struct etimer timer;

PROCESS(sink_node_process, "Healthcare Sink Node");
AUTOSTART_PROCESSES(&sink_node_process);

PROCESS_THREAD(sink_node_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Sink node started, listening on port %d\n", UDP_PORT);

  // Create UDP connection, listen on UDP_PORT
  udp_conn = udp_new(NULL, 0, NULL);
  udp_bind(udp_conn, UIP_HTONS(UDP_PORT));

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      if(uip_newdata()) {
        char msg[150];
        memset(msg, 0, sizeof(msg));

        size_t len = uip_datalen();
        if(len > 149) len = 149;
        strncpy(msg, (char *)uip_appdata, len);
        msg[len] = '\0';

        printf("⚠️  Received alert: %s\n", msg);

        // Blink LED non-blocking with timer
        leds_on(LEDS_GREEN);
        etimer_set(&timer, CLOCK_SECOND / 2);  // 0.5 seconds
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER && etimer_expired(&timer));
        leds_off(LEDS_GREEN);
      }
    }
  }

  PROCESS_END();
}
