#ifndef PROJECT_DEFS_H  #preventing the file being included mutiple times
#define PROJECT_DEFS_H  

#include <stdint.h>
#include <stdbool.h>


#define SYNC_WORD    0xAA
#define PAYLOAD_SIZE 160

typedef enum {
    STATE_BOOT,         #0
    STATE_IDLE,         #1
    STATE_ACTIVE_COMM,  #2
    STATE_ERROR_HALT    #3
} SystemState_t;    #to help with system serial communication insteaf of just numbers


typedef struct __attribute__((packed)) {
    uint8_t  sync_word;
    uint16_t packet_id;
    uint8_t  flags;
    uint8_t  payload[PAYLOAD_SIZE];
    uint16_t crc16;
} SecurePacket_t;   #defines and ensures packets are sent back to back without any tampering in other layers


_Static_assert(sizeof(SecurePacket_t) == 166, "Packet size mismatch");


#endif