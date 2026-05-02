

#include <stdio.h>
#include "timing.h"
#include "project_defs.h"

SecurePacket_t TX_packet;
SecurePacket_t RX_packet;



int main() {
time_init();
audio_init();
crypto_init();
transport_init();





while(1) {
    if(time_frame_ready()) {
        time_clear_frame_flag();


        SysHealth_MarkTaskStart(TASK_AUDIO);
        audio_get_frame(TX_packet.payload);     //this packet is coming from the audio module that is constantly filling a buffer with the values and only sending 160 bytes at a time, so we can be sure that the payload is always 160 bytes and ready to be sent when the frame is ready
        SysHealth_MarkTaskEnd(TASK_AUDIO);

        SysHealth_MarkTaskStart(TASK_CRYPTO);
        encrypt_packet((uint8_t *)TX_packet.payload);     //this is only looking at the payload, the rest of the packet is not encryped bcs crc and word sync cant be encrypted otherswise the packet will be rejected by the receiver, so only the payload is encrypted, while the rest of the packet is completed in the transport layer such as the packet num etc
        SysHealth_MarkTaskEnd(TASK_CRYPTO);

        SysHealth_MarkTaskStart(TASK_TRANSPORT_TX);
        transport_send((uint8_t *)&TX_packet, PACKET_SIZE); //all the transporter does is add the packet header such as the crc and sync word that will be found in the project_defs file, and will finalize the packet as a whole accoridngly and prepare for communication.
        SysHealth_MarkTaskEnd(TASK_TRANSPORT_TX);
    }
    if(transport_packet_received()){
        SysHealth_MarkTaskStart(TASK_TRANSPORT_RX);
        transport_receive((uint8_t *)&RX_packet, PACKET_SIZE);
        SysHealth_CalculateCRC(RX_packet.payload, PAYLOAD_SIZE); //this is to check if the packet is valid or not, if the crc is not correct then we will just discard the packet and not process it further, but if the crc is correct then we will proceed with decrypting the payload and playing the audio.
        SysHealth_MarkTaskEnd(TASK_TRANSPORT_RX);

        
        SysHealth_MarkTaskStart(TASK_CRYPTO);
        decrypt_packet((uint8_t *)RX_packet.payload);
        SysHealth_MarkTaskEnd(TASK_CRYPTO);

        SysHealth_MarkTaskStart(TASK_PLAYBACK);
        play_audio(RX_packet.payload);
        SysHealth_MarkTaskEnd(TASK_PLAYBACK);
    }
}



}//main ending
