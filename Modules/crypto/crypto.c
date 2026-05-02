#include "crypto.h"
#include <string.h> // Once the compiler path is fixed, this will work

/* Symmetric key for XOR encryption */
static const uint8_t SECRET_KEY[] = { 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89 };
#define KEY_SIZE sizeof(SECRET_KEY)

/**
 * Initializes the crypto module as declared in crypto.h
 */
void crypto_init(void) {
    // Logic for key initialization or hardware setup goes here
}

/**
 * Encrypts payload in-place inside SecurePacket_t[cite: 3]
 * Only modifies the payload array (160 bytes)[cite: 1, 3]
 */
void encrypt_packet(SecurePacket_t *packet) {
    if (packet == NULL) return;

    for (uint16_t i = 0; i < PAYLOAD_SIZE; i++) {
        // Simple XOR logic is fast enough for the 20ms audio frame window
        packet->payload[i] ^= SECRET_KEY[i % KEY_SIZE];
    }
}

/**
 * Decrypts payload in-place inside SecurePacket_t[cite: 3]
 * XOR is symmetric; repeating the operation decrypts the data
 */
void decrypt_packet(SecurePacket_t *packet) {
    encrypt_packet(packet);
}

/**
 * Validates that the data is not just a flat buffer of identical bytes[cite: 3]
 */
int crypto_validate_encryption(uint8_t *data, uint16_t len) {
    if (data == NULL || len < 2) return 0;
    for (uint16_t i = 1; i < len; i++) {
        if (data[i] != data[0]) return 1;
    }
    return 0;
}