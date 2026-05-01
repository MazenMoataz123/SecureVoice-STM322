#include "transport.h"
#include <string.h>

// ===== INTERNAL STATE =====
static uint16_t packet_counter = 0;

static uint8_t rx_stream_buffer[256];
static uint16_t rx_index = 0;

static int packet_ready = 0;
static SecurePacket_t rx_packet_internal;
static uint8_t virtual_wire[1024];
static int write_idx = 0;
static int read_idx = 0;


// ===== MOCK UART (REPLACE WITH HAL) =====
void uart_send_bytes(uint8_t *data, uint16_t len) {
    for (int i = 0; i < len; i++) {
        virtual_wire[write_idx++] = data[i];
    }
}

int uart_receive_byte(uint8_t *byte) {
    if (read_idx < write_idx) {
        *byte = virtual_wire[read_idx++];
        return 1;
    }
    return 0;
}


// ===== CRC (simple example, replace with your SysHealth if needed) =====
static uint16_t calculate_crc(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;

    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}


// ===== INIT =====
void transport_init(void) {
    packet_counter = 0;
    rx_index = 0;
    packet_ready = 0;
}


// ===== SEND =====
void transport_send(SecurePacket_t *packet) {

    // Fill header
    packet->sync_byte = SYNC_WORD;
    packet->packet_id = packet_counter++;
    packet->flags = 0x00;

    // Compute CRC (exclude sync + crc field)
    packet->crc16 = calculate_crc(
        (uint8_t *)&packet->packet_id,
        sizeof(SecurePacket_t) - 1 - 2
    );

    // Send via UART
    uart_send_bytes((uint8_t *)packet, sizeof(SecurePacket_t));
}


// ===== RECEIVE STREAM PROCESSING =====
static void process_incoming_byte(uint8_t byte) {

    rx_stream_buffer[rx_index++] = byte;

    // Prevent overflow
    if (rx_index >= sizeof(rx_stream_buffer)) {
        rx_index = 0;
    }

    // Try to detect packet
    for (int i = 0; i < rx_index; i++) {

        if (rx_stream_buffer[i] == SYNC_WORD) {

            if ((rx_index - i) >= sizeof(SecurePacket_t)) {

                memcpy(&rx_packet_internal,
                       &rx_stream_buffer[i],
                       sizeof(SecurePacket_t));

                // Validate CRC
                uint16_t crc = calculate_crc(
                    (uint8_t *)&rx_packet_internal.packet_id,
                    sizeof(SecurePacket_t) - 1 - 2
                );

                if (crc == rx_packet_internal.crc16) {
                    packet_ready = 1;
                }

                // Shift buffer
                memmove(rx_stream_buffer,
                        &rx_stream_buffer[i + sizeof(SecurePacket_t)],
                        rx_index - (i + sizeof(SecurePacket_t)));

                rx_index -= (i + sizeof(SecurePacket_t));
                return;
            }
        }
    }
}


// ===== RECEIVE API =====
int transport_receive(SecurePacket_t *packet) {

    if (!packet_ready)
        return 0;

    memcpy(packet, &rx_packet_internal, sizeof(SecurePacket_t));
    packet_ready = 0;

    return 1;
}


// ===== CHECK IF PACKET READY =====
int transport_packet_received(void) {
    return packet_ready;
}


// ===== POLLING FUNCTION (CALL FREQUENTLY) =====
void transport_process_rx(void) {
    uint8_t byte;

    while (uart_receive_byte(&byte)) {
        process_incoming_byte(byte);
    }
}