#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include "project_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void crypto_init(void);

void encrypt_packet(SecurePacket_t *packet);

void decrypt_packet(SecurePacket_t *packet);

int crypto_validate_encryption(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif