#include "app.h"

#include "timing.h"
#include "audio.h"
#include "crypto.h"
#include "transport.h"
#include "sys_health.h"
#include "project_defs.h"

SecurePacket_t TX_packet;
SecurePacket_t RX_packet;

void app_init(void)
{
    time_init();
    audio_init();
    crypto_init();
    transport_init();
}

void app_loop(void)
{
    if(time_frame_ready())
    {
        time_clear_frame_flag();

        SysHealth_MarkTaskStart(TASK_AUDIO);

        audio_get_frame(TX_packet.payload);

        SysHealth_MarkTaskEnd(TASK_AUDIO);

        SysHealth_MarkTaskStart(TASK_CRYPTO);

        encrypt_packet(&TX_packet);

        SysHealth_MarkTaskEnd(TASK_CRYPTO);

        SysHealth_MarkTaskStart(TASK_TRANSPORT_TX);

        transport_send(&TX_packet);

        SysHealth_MarkTaskEnd(TASK_TRANSPORT_TX);
    }

    if(transport_packet_received())
    {
        SysHealth_MarkTaskStart(TASK_TRANSPORT_RX);

        transport_receive(&RX_packet);

        SysHealth_CalculateCRC(RX_packet.payload, PAYLOAD_SIZE);

        SysHealth_MarkTaskEnd(TASK_TRANSPORT_RX);

        SysHealth_MarkTaskStart(TASK_CRYPTO);

        decrypt_packet(&RX_packet);

        SysHealth_MarkTaskEnd(TASK_CRYPTO);

        SysHealth_MarkTaskStart(TASK_PLAYBACK);

        play_audio(RX_packet.payload);

        SysHealth_MarkTaskEnd(TASK_PLAYBACK);
    }
}