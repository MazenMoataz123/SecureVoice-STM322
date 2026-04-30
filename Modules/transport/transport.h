#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdint.h>
#include "project_defs.h"

/*
 * TRANSPORT MODULE
 * Responsibility:
 * - Send/receive raw packets over UART (HC-05 Bluetooth)
 * - Handle sync detection
 * - Handle byte streaming only (NO encryption logic)
 */

void transport_init(void);

/*
 * Sends raw packet over UART (DMA preferred)
 */
void transport_send(uint8_t *data, uint16_t len);

/*
 * Receives raw packet into buffer
 * Returns number of bytes received
 */
int transport_receive(uint8_t *buffer, uint16_t len);

/*
 * Searches for sync byte in incoming stream
 * Returns index or -1 if not found
 */
int transport_find_sync(uint8_t *stream, uint16_t len);

#endif