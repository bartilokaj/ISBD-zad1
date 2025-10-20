#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#define CRC64_POLY 0x42F0E1EBA9EA3693ULL

static uint64_t crc64_table[256];
static int crc64_table_init_done = 0;

static void crc64_init(void)
{
    if (crc64_table_init_done) return;

    for (int i = 0; i < 256; ++i) {
        uint64_t crc = (uint64_t)i << 56;
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000000000000000ULL) {
                crc = (crc << 1) ^ CRC64_POLY;
            } else {
                crc <<= 1;
            }
        }
        crc64_table[i] = crc;
    }

    crc64_table_init_done = 1;
}

uint64_t crc64_update(uint64_t crc, const unsigned char *buf, size_t len)
{
    if (!crc64_table_init_done) crc64_init();

    while (len--) {
        uint8_t idx = (uint8_t)((crc >> 56) ^ *buf++);
        crc = (crc << 8) ^ crc64_table[idx];
    }
    return crc;
}

uint64_t crc64(const unsigned char *buf, size_t len)
{
    return crc64_update(0ULL, buf, len);
}