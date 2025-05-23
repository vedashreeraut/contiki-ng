//vapis
#include "contiki.h"
#include "contiki-net.h"
#include "sys/etimer.h"
#include "random.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#define TEMP_THRESHOLD 38
#define HEART_THRESHOLD 100
#define SPO2_THRESHOLD 92
#define RESP_THRESHOLD 20
#define BP_SYS_THRESHOLD 130
#define BP_DIA_THRESHOLD 85

#define UDP_PORT 1234

static struct uip_udp_conn *udp_conn;

PROCESS(alert_node_process, "Healthcare Alert Node");
AUTOSTART_PROCESSES(&alert_node_process);

PROCESS_THREAD(alert_node_process, ev, data)
{
  static struct etimer timer;
  static int temp, heart_rate, spo2, resp_rate, bp_sys, bp_dia;
  static char msg[150];

  PROCESS_BEGIN();

  udp_conn = udp_new(NULL, UIP_HTONS(UDP_PORT), NULL);
  udp_bind(udp_conn, UIP_HTONS(UDP_PORT));

  printf("Alert node started, sending on port %d\n", UDP_PORT);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    temp = 36 + (random_rand() % 5);
    heart_rate = 70 + (random_rand() % 50);
    spo2 = 90 + (random_rand() % 11);
    resp_rate = 12 + (random_rand() % 10); // 12–21
    bp_sys = 110 + (random_rand() % 30);   // 110–139
    bp_dia = 70 + (random_rand() % 20);    // 70–89

    printf("Vitals: Temp=%d HR=%d SpO2=%d%% Resp=%d BP=%d/%d\n",
           temp, heart_rate, spo2, resp_rate, bp_sys, bp_dia);

    int is_alert = (temp > TEMP_THRESHOLD || heart_rate > HEART_THRESHOLD ||
                    spo2 < SPO2_THRESHOLD || resp_rate > RESP_THRESHOLD ||
                    bp_sys > BP_SYS_THRESHOLD || bp_dia > BP_DIA_THRESHOLD);

    const char *severity = (temp > TEMP_THRESHOLD + 1 || heart_rate > HEART_THRESHOLD + 10 ||
                            spo2 < SPO2_THRESHOLD - 2 || resp_rate > RESP_THRESHOLD + 2 ||
                            bp_sys > BP_SYS_THRESHOLD + 10 || bp_dia > BP_DIA_THRESHOLD + 5) ? "CRITICAL" : "MODERATE";

    if(is_alert) {
      snprintf(msg, sizeof(msg), "ALERT #%lu [%s] @%lu: Temp=%d HR=%d SpO2=%d Resp=%d BP=%d/%d",
               clock_seconds(), severity, clock_time(),
               temp, heart_rate, spo2, resp_rate, bp_sys, bp_dia);

      uip_create_linklocal_allnodes_mcast(&udp_conn->ripaddr);
      uip_udp_packet_send(udp_conn, msg, strlen(msg));
      uip_create_unspecified(&udp_conn->ripaddr);

      leds_on(LEDS_RED);
      etimer_set(&timer, CLOCK_SECOND / 2);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      leds_off(LEDS_RED);

      printf("Sent alert: %s\n", msg);
    } else {
      leds_on(LEDS_GREEN);
      etimer_set(&timer, CLOCK_SECOND / 2);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      leds_off(LEDS_GREEN);
    }
  }

  PROCESS_END();
}
