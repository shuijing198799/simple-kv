#ifndef __HASH_H
#define __HASH_H
#include "str.h"
#include <stdint.h>

uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);

#endif
