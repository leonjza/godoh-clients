#ifndef GODOH_CRC32_H
#define GODOH_CRC32_H

#include <sys/types.h>
#include <stdint.h>

void crc32(const void *data, size_t n_bytes, uint32_t* crc);

#endif //GODOH_CRC32_H
