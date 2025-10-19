#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint64_t crc64_update (uint64_t crc, const unsigned char *buf, size_t len);

#endif