#ifndef CANNY_H
#define CANNY_H

#include "image.h"

image canny_image(image m, int reduce_noise);

void canny_sobel_image(image in, int* G, int* theta);
void canny_nms(image* m, int* G, int* theta);
void canny_estimate_threshold(image* m, int* low, int* high);
void canny_hysteresis(int high, int low, image* in, image* out);

#endif
