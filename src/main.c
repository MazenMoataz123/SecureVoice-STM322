#ifndef TIMING_H
#define TIMING_H

#include <stdio.h>
#include "timing.c"
#include "project_defs.h"

SecurePacket_t TX_packet;
SecurePacket_t RX_packet;

time_init();
audio_init();
crypto_init();
transport_init();





while(1) {
    if(time_frame_ready()) {
        time_clear_frame_flag();
        
        audio_get_frame(TX_packet.payload);     //this packet is coming from the audio module that is constantly filling a buffer with the values and only sending 160 bytes at a time, so we can be sure that the payload is always 160 bytes and ready to be sent when the frame is ready
        encrypt_packet(*TX_packet.payload);     //this is only looking at the payload, the rest of the packet is not encryped bcs crc and word sync cant be encrypted otherswise the packet will be rejected by the receiver, so only the payload is encrypted, while the rest of the packet is completed in the transport layer such as the packet num etc
        transport_send((uint8_t *)&TX_packet, PACKET_SIZE);   //all the transporter does is add the packet header such as the crc and sync word that will be found in the project_defs file, and will finalize the packet as a whole accoridngly and prepare for communication.

    }
    if(transport_packet_received()){
        decrypt_packet(*RX_packet);
        play_audio(RX_packet.payload);
    }
}




