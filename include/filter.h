#ifndef FILTER_H
#define FILTER_H

#include "image.h"

image convolve_image(image m, image filter, int preserve);

image equalize_histogram(image m);

image* sobel_image(image m);
image sharpen_image(image m);
image smoothen_image(image m);
image gaussian_noise_reduce(image grayscale);

// morphological transformations
image dilate_image(image m, int times);
image erode_image(image m, int times);
image hilditch_thin_image(image m);

#endif
