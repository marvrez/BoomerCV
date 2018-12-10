#ifndef FILTER_H
#define FILTER_H

#include "image.h"

image gaussian_noise_reduce(image grayscale);
image equalize_histogram(image m);

image dilate_image(image m, int times);
image erode_image(image m, int times);

#endif
