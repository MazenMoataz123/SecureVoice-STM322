#include "transport.h"
#include "stm32f1xx_hal.h" 
#include <string.h>

/* ===================== CONFIG ===================== */

extern UART_HandleTypeDef huart1;

#define RX_BUFFER_SIZE 256
#define MAX_RETRIES    3
#define ACK_TIMEOUT    100000   // crude delay loop


/* ===================== INTERNAL ===================== */

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint16_t rx_index = 0;

static SecurePacket_t rx_packet;
static volatile int packet_ready = 0;

static uint16_t packet_counter = 0;

static uint8_t rx_byte;


/* ===================== CRC ===================== */

static uint16_t calc_crc(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}


/* ===================== INIT ===================== */

void transport_init(void)
{
    rx_index = 0;
    packet_ready = 0;
    packet_counter = 0;

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}


/* ===================== LOW LEVEL SEND ===================== */

static void send_raw(SecurePacket_t *packet)
{
    HAL_UART_Transmit(&huart1,
                      (uint8_t *)packet,
                      sizeof(SecurePacket_t),
                      HAL_MAX_DELAY);
}


/* ===================== SEND WITH ACK ===================== */

int transport_send(SecurePacket_t *packet)
{
    packet->sync_byte = SYNC_WORD;
    packet->packet_id = packet_counter++;
    packet->flags = FLAG_DATA;

    packet->crc16 = calc_crc(
        (uint8_t *)&packet->packet_id,
        sizeof(SecurePacket_t) - 1 - 2
    );

    for (int retry = 0; retry < MAX_RETRIES; retry++) {

        send_raw(packet);

        // wait for ACK
        int timeout = ACK_TIMEOUT;
        while (timeout--) {

            if (packet_ready &&
                (rx_packet.flags & FLAG_ACK) &&
                (rx_packet.packet_id == packet->packet_id)) {

                packet_ready = 0;
                return 1; // success
            }
        }
    }

    return 0; // failed
}


/* ===================== HANDSHAKE ===================== */

int transport_handshake(void)
{
    SecurePacket_t hello;

    memset(&hello, 0, sizeof(hello));

    hello.sync_byte = SYNC_WORD;
    hello.packet_id = 0;
    hello.flags = FLAG_HELLO;

    hello.crc16 = calc_crc(
        (uint8_t *)&hello.packet_id,
        sizeof(SecurePacket_t) - 1 - 2
    );

    send_raw(&hello);

    int timeout = ACK_TIMEOUT;

    while (timeout--) {
        if (packet_ready && (rx_packet.flags & FLAG_ACK)) {
            packet_ready = 0;
            return 1;
        }
    }

    return 0;
}


/* ===================== SYNC FIND ===================== */

int transport_find_sync(uint8_t *stream, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (stream[i] == SYNC_WORD)
            return i;
    }
    return -1;
}


/* ===================== BYTE PROCESS ===================== */

static void process_byte(uint8_t byte)
{
    rx_buffer[rx_index++] = byte;

    if (rx_index >= RX_BUFFER_SIZE)
        rx_index = 0;

    int sync_pos = transport_find_sync(rx_buffer, rx_index);

    if (sync_pos >= 0) {

        if ((rx_index - sync_pos) >= sizeof(SecurePacket_t)) {

            memcpy(&rx_packet,
                   &rx_buffer[sync_pos],
                   sizeof(SecurePacket_t));

            uint16_t crc = calc_crc(
                (uint8_t *)&rx_packet.packet_id,
                sizeof(SecurePacket_t) - 1 - 2
            );

            if (crc == rx_packet.crc16) {

                // AUTO SEND ACK
                if (rx_packet.flags & FLAG_DATA) {

                    SecurePacket_t ack;
                    memset(&ack, 0, sizeof(ack));

                    ack.sync_byte = SYNC_WORD;
                    ack.packet_id = rx_packet.packet_id;
                    ack.flags = FLAG_ACK;

                    ack.crc16 = calc_crc(
                        (uint8_t *)&ack.packet_id,
                        sizeof(SecurePacket_t) - 1 - 2
                    );

                    send_raw(&ack);
                }

                packet_ready = 1;
            }

            uint16_t remaining = rx_index - (sync_pos + sizeof(SecurePacket_t));

            memmove(rx_buffer,
                    &rx_buffer[sync_pos + sizeof(SecurePacket_t)],
                    remaining);

            rx_index = remaining;
        }
    }
}


/* ===================== UART CALLBACK ===================== */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {

        process_byte(rx_byte);

        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}


/* ===================== RECEIVE ===================== */

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


/* ===================== OPTIONAL ===================== */

void transport_process_rx(void)
{
    // Not needed (interrupt-driven)
}
