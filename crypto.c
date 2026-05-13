#include "crypto.h"
#include <stddef.h>

/*
 * Simple symmetric XOR key.
 *
 * This is useful for testing the secure voice pipeline.
 * It is NOT strong real-world encryption.
 */
static const uint8_t SECRET_KEY[] = {
    0xAB, 0xCD, 0xEF, 0x01,
    0x23, 0x45, 0x67, 0x89
};

#define KEY_SIZE ((uint16_t)(sizeof(SECRET_KEY) / sizeof(SECRET_KEY[0])))

void crypto_init(void)
{
    /*
     * Nothing is needed for simple XOR encryption.
     * If we upgrade to AES later, key setup can happen here.
     */
}

void encrypt_packet(SecurePacket_t *packet)
{
    if (packet == NULL) {
        return;
    }

    for (uint16_t i = 0; i < PAYLOAD_SIZE; i++) {
        packet->payload[i] ^= SECRET_KEY[i % KEY_SIZE];
    }
}

void decrypt_packet(SecurePacket_t *packet)
{
    /*
     * XOR encryption is symmetric.
     *
     * That means:
     * plaintext ^ key = ciphertext
     * ciphertext ^ key = plaintext
     *
     * So decryption is the same operation as encryption.
     */
    encrypt_packet(packet);
}

int crypto_validate_encryption(uint8_t *data, uint16_t len)
{
    if (data == NULL || len < 2) {
        return 0;
    }

    for (uint16_t i = 1; i < len; i++) {
        if (data[i] != data[0]) {
            return 1;
        }
    }

    return 0;
}