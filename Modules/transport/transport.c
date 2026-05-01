#include "transport.h"
#include <string.h>

/* ===================== CONFIG ===================== */

extern UART_HandleTypeDef huart1;   // Provided by STM32 HAL

#define RX_BUFFER_SIZE 256


/* ===================== INTERNAL STATE ===================== */

static uint16_t packet_counter = 0;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint16_t rx_index = 0;

static SecurePacket_t rx_packet;
static volatile int packet_ready = 0;


/* ===================== CRC FUNCTION ===================== */

static uint16_t transport_calculate_crc(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}


/* ===================== INIT ===================== */

void transport_init(void)
{
    packet_counter = 0;
    rx_index = 0;
    packet_ready = 0;

    // Start UART RX interrupt (1 byte at a time)
    static uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}


/* ===================== TRANSMIT ===================== */

void transport_send(SecurePacket_t *packet)
{
    // Fill header
    packet->sync_byte = SYNC_WORD;
    packet->packet_id = packet_counter++;
    packet->flags = 0x00;

    // Compute CRC (exclude sync + crc field)
    packet->crc16 = transport_calculate_crc(
        (uint8_t *)&packet->packet_id,
        sizeof(SecurePacket_t) - 1 - 2
    );

    // Send packet via UART (blocking or DMA)
    HAL_UART_Transmit(&huart1, (uint8_t *)packet, sizeof(SecurePacket_t), HAL_MAX_DELAY);
}


/* ===================== SYNC SEARCH ===================== */

int transport_find_sync(uint8_t *stream, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (stream[i] == SYNC_WORD) {
            return i;
        }
    }
    return -1;
}


/* ===================== INTERNAL PARSER ===================== */

static void transport_process_byte(uint8_t byte)
{
    rx_buffer[rx_index++] = byte;

    // Prevent overflow
    if (rx_index >= RX_BUFFER_SIZE)
        rx_index = 0;

    // Try to find sync
    int sync_pos = transport_find_sync(rx_buffer, rx_index);

    if (sync_pos >= 0) {

        // Check if full packet available
        if ((rx_index - sync_pos) >= sizeof(SecurePacket_t)) {

            memcpy(&rx_packet,
                   &rx_buffer[sync_pos],
                   sizeof(SecurePacket_t));

            // Validate CRC
            uint16_t crc = transport_calculate_crc(
                (uint8_t *)&rx_packet.packet_id,
                sizeof(SecurePacket_t) - 1 - 2
            );

            if (crc == rx_packet.crc16) {
                packet_ready = 1;
            }

            // Remove processed data from buffer
            uint16_t remaining = rx_index - (sync_pos + sizeof(SecurePacket_t));

            memmove(rx_buffer,
                    &rx_buffer[sync_pos + sizeof(SecurePacket_t)],
                    remaining);

            rx_index = remaining;
        }
    }
}


/* ===================== UART RX CALLBACK ===================== */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {

        static uint8_t rx_byte;

        // Process received byte
        transport_process_byte(rx_byte);

        // Restart reception
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}


/* ===================== RX API ===================== */

int transport_packet_received(void)
{
    return packet_ready;
}


int transport_receive(SecurePacket_t *packet)
{
    if (!packet_ready)
        return 0;

    memcpy(packet, &rx_packet, sizeof(SecurePacket_t));
    packet_ready = 0;

    return 1;
}


/* ===================== OPTIONAL POLLING ===================== */

void transport_process_rx(void)
{
    // Not needed if using interrupt mode
}
