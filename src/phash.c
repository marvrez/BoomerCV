#include "phash.h"

#include "filter.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

bool dct_transform(float vector[], size_t len);

int float_compare(const void * a, const void * b)
{
    float fa = *(const float*) a;
    float fb = *(const float*) b;
    return (fa > fb) - (fa < fb);
}

static inline int hamming_distance(uint64_t a, uint64_t b)
{
    uint64_t x = a^b;
    int count = 0;
    while(x) {
        x &= x - 1;
        count++;
    }
    return count;
}

uint64_t phash(image m)
{
    image gray = m.c == 1 ? copy_image(m) : rgb_to_grayscale(m);
    image smoothened = smoothen_image(gray, 3);
    image resized = bilinear_resize(smoothened, 32, 32);

    size_t len = resized.w*resized.h;
    dct_transform(resized.data, len);

    float mean = 0.f;
    for(int y = 1; y <= 8; ++y) {
        for(int x = 1; x <= 8; ++x) {
            mean += resized.data[x + y*resized.w];
        }
    }
    mean /= 64.f;

	uint64_t result = 0x0, bit_idx = 0x1;
    for(int y = 1; y <= 8; ++y) {
        for(int x = 1; x <= 8; ++x) {
            float val = resized.data[x + y*resized.w];
            if(val > mean) result |= bit_idx;
            bit_idx <<= 1;
        }
    }
    free_image(&gray);
    free_image(&smoothened);
    free_image(&resized);

    return result;
}

int phash_compare(uint64_t a, uint64_t b)
{
    return hamming_distance(a, b);
}

/*
 * Fast discrete cosine transform algorithms (C)
 *
 * Copyright (c) 2017 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/fast-discrete-cosine-transform-algorithms
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */
static void forward_transform(float vector[], float temp[], size_t len) 
{
    if (len == 1) return;
    size_t halfLen = len / 2;
    for(size_t i = 0; i < halfLen; i++) {
        float x = vector[i];
        float y = vector[len - 1 - i];
        temp[i] = x + y;
        temp[i + halfLen] = (x - y) / (cos((i + 0.5) * M_PI / len) * 2);
    }
    forward_transform(temp, vector, halfLen);
    forward_transform(&temp[halfLen], vector, halfLen);
    for(size_t i = 0; i < halfLen - 1; i++) {
        vector[i * 2 + 0] = temp[i];
        vector[i * 2 + 1] = temp[i + halfLen] + temp[i + halfLen + 1];
    }
    vector[len - 2] = temp[halfLen - 1];
    vector[len - 1] = temp[len - 1];
}

// DCT type II, unscaled. Algorithm by Byeong Gi Lee, 1984.
// See: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.118.3056&rep=rep1&type=pdf#page=34
bool dct_transform(float vector[], size_t len) 
{
    if (len > 0 && (len & (len - 1)) != 0) return false;  // Length is not power of 2
    if (SIZE_MAX / sizeof(float) < len) return false;
    float* temp = malloc(len * sizeof(float));
    if (temp == NULL) return false;
    forward_transform(vector, temp, len);
    free(temp);
    return true;
}
