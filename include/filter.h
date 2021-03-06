#ifndef FILTER_H
#define FILTER_H

#include "image.h"

image convolve_image(image m, image filter, int preserve);

image equalize_histogram(image m);

image make_gx_filter();
image make_gy_filter();

image colorize_sobel(image m);
image* sobel_image(image m);
image sharpen_image(image m);
image smoothen_image(image m, int w);
image gaussian_noise_reduce(image m, float sigma);

// morphological transformations
image dilate_image(image m, int times);
image erode_image(image m, int times);
image skeletonize_image(image m);

#endif
