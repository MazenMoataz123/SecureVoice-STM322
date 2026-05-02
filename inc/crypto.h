#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include "project_defs.h"

/*
 * CRYPTO MODULE
 * Responsibility:
 * - Encrypt / decrypt packet payload ONLY
 * - Must NOT modify headers or CRC fields
 */

void crypto_init(void);

/*
 * Encrypts payload in-place inside SecurePacket_t
 * Only payload[] is modified
 */
void encrypt_packet(SecurePacket_t *packet);

/*
 * Decrypts payload in-place inside SecurePacket_t
 */
void decrypt_packet(SecurePacket_t *packet);

/*
 * Optional validation helper:
 * returns 1 if encryption appears valid (non-trivial entropy check)
 */
int crypto_validate_encryption(uint8_t *data, uint16_t len);

#endif