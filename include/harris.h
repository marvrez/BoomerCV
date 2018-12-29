#ifndef HARRIS_H
#define HARRIS_H

#include "image.h"

// A descriptor for a point in an image.
// point p: x,y coordinates of the image pixel.
// int n: the number of floating point values in the descriptor.
// float* data: the descriptor for the pixel.
typedef struct {
    point p;
    int n;
    float* data;
} descriptor;

image make_structure_matrix(image m, float sigma);
image harris_cornerness_response(image S);
image harris_nms_image(image m, int w);
descriptor* harris_corner_detector(image m, float sigma, float thresh, int nms, int* n);

void draw_corners(image* m, descriptor* d, int n);

void free_descriptors(descriptor* d, int n);

#endif
