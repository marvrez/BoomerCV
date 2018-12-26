#ifndef CANNY_H
#define CANNY_H

#include "image.h"

image canny_image(image m, int reduce_noise);

void canny_sobel_image(image in, int* G, int* theta);
void canny_nms(int* G, int* theta, image* out);
void canny_estimate_threshold(image m, int* weak_threshold, int* strong_threshold);
void canny_hysteresis(int weak_threshold, int strong_threshold, image in, image* out);

#endif
