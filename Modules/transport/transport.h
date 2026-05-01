#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include "../../inc/project_defs.h"

void transport_init(void);

/* Send full packet (transport fills headers + CRC) */
void transport_send(uint8_t *packet, uint16_t size);

/* RX processing (must be called continuously) */
void transport_process_rx(void);

/* Check if packet ready */
int transport_packet_received(void);

/* Get received packet */
int transport_receive(uint8_t *packet, uint16_t size);

/* Sync search (utility) */
int transport_find_sync(uint8_t *stream, uint16_t len);

#endif
