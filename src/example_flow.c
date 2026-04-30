#include "audio.h"
#include "crypto.h"
#include "transport.h"
#include "project_defs.h"

#include <stdio.h>

int main(void)
{
    SecurePacket_t pkt;

    printf("System Start\n");

    // Step 1: Audio fills payload
    audio_get_frame(pkt.payload);

    printf("Audio frame ready\n");

    // Step 2: Encrypt
    encrypt_packet(&pkt);

    printf("Packet encrypted\n");

    // Step 3: Add metadata
    pkt.sync_word = SYNC_WORD;
    pkt.packet_id = 1;

    // Step 4: Send
    transport_send((uint8_t*)&pkt, sizeof(pkt));

    printf("Packet sent\n");

    return 0;
}
