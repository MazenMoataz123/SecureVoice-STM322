#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include "project_defs.h"
#include "config.h"

/*
 * AUDIO MODULE
 * Responsibility:
 * - Capture raw audio (mic / simulated input)
 * - Compress into 8-bit G.711 A-law
 * - Output fixed-size frame (PAYLOAD_SIZE)
 */

void audio_init(void);

/*
 * Fills buffer with exactly PAYLOAD_SIZE bytes of encoded audio.
 * Output format: G.711 A-law (8-bit per sample)
 *
 * Must be deterministic and real-time safe (< frame deadline).
 */
void audio_get_frame(uint8_t *buffer);

/*
 * Optional debug hook:
 * Returns 1 if audio source is healthy, 0 otherwise
 */
int audio_health_check(void);

#endif