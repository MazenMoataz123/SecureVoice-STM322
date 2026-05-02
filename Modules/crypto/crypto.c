#include "crypto.h"
#include <string.h>

/* =========================================================
 * Internal crypto key storage
 * ========================================================= */
static uint8_t crypto_key[CRYPTO_KEY_SIZE];
static bool crypto_initialized = false;


/* =========================================================
 * crypto_init()
 * Initializes fixed internal XOR stream key
 * ========================================================= */
void crypto_init(void)
{
    crypto_key[0] = 0x13;
    crypto_key[1] = 0x57;
    crypto_key[2] = 0x9B;
    crypto_key[3] = 0xDF;
    crypto_key[4] = 0x24;
    crypto_key[5] = 0x68;
    crypto_key[6] = 0xAC;
    crypto_key[7] = 0xF0;

    crypto_initialized = true;
}


/* =========================================================
 * crypto_apply_stream()
 * Applies XOR rotating stream cipher to raw data buffer
 * Same function works for encrypt and decrypt
 * ========================================================= */
void crypto_apply_stream(uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0)
        return;

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] ^= crypto_key[i % CRYPTO_KEY_SIZE];
        data[i] ^= (uint8_t)(i * 7);   /* position scrambling */
    }
}


/* =========================================================
 * encrypt_packet()
 * Encrypt only valid payload bytes
 * ========================================================= */
bool encrypt_packet(SecurePacket_t *packet)
{
    if (!crypto_initialized)
        return false;

    if (packet == NULL)
        return false;

    if (packet->payload_length == 0 || packet->payload_length > PAYLOAD_SIZE)
        return false;

    crypto_apply_stream(packet->payload, packet->payload_length);

    packet->flags |= FLAG_ENCRYPTED;

    return true;
}


/* =========================================================
 * decrypt_packet()
 * Decrypt only valid payload bytes
 * ========================================================= */
bool decrypt_packet(SecurePacket_t *packet)
{
    if (!crypto_initialized)
        return false;

    if (packet == NULL)
        return false;

    if (packet->payload_length == 0 || packet->payload_length > PAYLOAD_SIZE)
        return false;

    crypto_apply_stream(packet->payload, packet->payload_length);

    packet->flags &= ~FLAG_ENCRYPTED;

    return true;
}


/* =========================================================
 * crypto_validate_encryption()
 * Simple entropy-like validation for testing/demo
 * Returns 1 if data looks sufficiently randomized
 * ========================================================= */
int crypto_validate_encryption(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len < 4)
        return 0;

    uint16_t repeated = 0;

    for (uint16_t i = 1; i < len; i++)
    {
        if (data[i] == data[i - 1])
            repeated++;
    }

    /* if too many adjacent repeated bytes, data looks weak/plain */
    if (repeated > (len / 3))
        return 0;

    return 1;
}