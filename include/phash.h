#ifndef PHASH_H
#define PHASH_H

#include "image.h"

#include <stdint.h>

uint64_t phash(image m);
int phash_compare(uint64_t a, uint64_t b);

#endif
