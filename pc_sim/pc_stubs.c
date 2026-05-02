#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/project_defs.h"

#define PC_SIM_MAX_FRAMES 5

static SecurePacket_t loopback_packet;
static int packet_ready;
static uint16_t packet_counter;
static int frame_number;

static uint16_t pc_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t j;

    for (i = 0; i < len; i++) {
        crc ^= data[i];

        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

void time_init(void)
{
    frame_number = 0;
    printf("[PC SIM] timing initialized\n");
}

int time_frame_ready(void)
{
    if (frame_number >= PC_SIM_MAX_FRAMES) {
        printf("[PC SIM] completed %d frames, exiting simulation\n", PC_SIM_MAX_FRAMES);
        exit(0);
    }

    frame_number++;
    return 1;
}

void time_clear_frame_flag(void)
{
}

void audio_init(void)
{
    printf("[PC SIM] audio initialized with generated test frames\n");
}

void audio_get_frame(uint8_t *payload)
{
    int i;

    for (i = 0; i < PAYLOAD_SIZE; i++) {
        payload[i] = (uint8_t)((frame_number + i) & 0xFF);
    }

    printf("[PC SIM] captured frame %d\n", frame_number);
}

void crypto_init(void)
{
    printf("[PC SIM] crypto initialized with XOR test transform\n");
}

void encrypt_packet(uint8_t *payload)
{
    int i;

    for (i = 0; i < PAYLOAD_SIZE; i++) {
        payload[i] ^= 0x5A;
    }

    printf("[PC SIM] encrypted payload\n");
}

void decrypt_packet(uint8_t *payload)
{
    int i;

    for (i = 0; i < PAYLOAD_SIZE; i++) {
        payload[i] ^= 0x5A;
    }

    printf("[PC SIM] decrypted payload\n");
}

void transport_init(void)
{
    memset(&loopback_packet, 0, sizeof(loopback_packet));
    packet_ready = 0;
    packet_counter = 0;
    printf("[PC SIM] transport initialized in UART loopback mode\n");
}

int transport_send(uint8_t *packet_bytes, unsigned long packet_size)
{
    SecurePacket_t *packet = (SecurePacket_t *)packet_bytes;

    if (packet_size != sizeof(SecurePacket_t)) {
        printf("[PC SIM] warning: packet size is %lu, expected %lu\n",
               packet_size,
               (unsigned long)sizeof(SecurePacket_t));
    }

    packet->sync_byte = SYNC_WORD;
    packet->packet_id = packet_counter++;
    packet->flags = 0x01;
    packet->crc16 = pc_crc16((uint8_t *)&packet->packet_id,
                             (uint16_t)(sizeof(SecurePacket_t) - 1 - 2));

    memcpy(&loopback_packet, packet, sizeof(loopback_packet));
    packet_ready = 1;

    printf("[PC SIM] sent packet id=%u crc=0x%04X\n",
           packet->packet_id,
           packet->crc16);

    return 1;
}

int transport_packet_received(void)
{
    return packet_ready;
}

int transport_receive(uint8_t *packet_bytes, unsigned long packet_size)
{
    if (!packet_ready) {
        return 0;
    }

    if (packet_size < sizeof(SecurePacket_t)) {
        return 0;
    }

    memcpy(packet_bytes, &loopback_packet, sizeof(loopback_packet));
    packet_ready = 0;

    printf("[PC SIM] received loopback packet id=%u\n", loopback_packet.packet_id);
    return 1;
}

void play_audio(uint8_t *payload)
{
    printf("[PC SIM] played audio frame, first bytes: %02X %02X %02X %02X\n",
           payload[0],
           payload[1],
           payload[2],
           payload[3]);
}

void SysHealth_MarkTaskStart(uint8_t task_id)
{
    printf("[PC SIM] task %u start\n", task_id);
}

void SysHealth_MarkTaskEnd(uint8_t task_id)
{
    printf("[PC SIM] task %u end\n", task_id);
}

uint16_t SysHealth_CalculateCRC(uint8_t *data, uint16_t len)
{
    uint16_t crc = pc_crc16(data, len);

    printf("[PC SIM] payload crc check value=0x%04X\n", crc);
    return crc;
}
