#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>

PROCESS(sink_node_process, "Alert Sink Node");
AUTOSTART_PROCESSES(&sink_node_process);

static void recv_callback(struct broadcast_conn *c, const linkaddr_t *from, const uint8_t *data, uint16_t len) {
  printf("Received ALERT from %02x:%02x: %s\n", from->u8[0], from->u8[1], data);
}

static const struct broadcast_callbacks bc_callbacks = {recv_callback};
static struct broadcast_conn bc;

PROCESS_THREAD(sink_node_process, ev, data) {
  PROCESS_EXITHANDLER(broadcast_close(&bc);)
  PROCESS_BEGIN();

  broadcast_open(&bc, 129, &bc_callbacks);

  PROCESS_END();
}
