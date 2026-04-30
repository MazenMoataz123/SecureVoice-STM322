#ifndef TIMING_H
#define TIMING_H

#define SAMPLE_RATE_HZ        8000
#define FRAME_DURATION_MS     20

#define SAMPLES_PER_FRAME     ((SAMPLE_RATE_HZ * FRAME_DURATION_MS) / 1000)
#define BYTES_PER_FRAME       SAMPLES_PER_FRAME

#define FULL_DUPLEX_ENABLED   1

// Timing constraints (VERY IMPORTANT for STM32 real-time behavior)
#define FRAME_PERIOD_US       20000  // 20ms

#endif