#ifndef FLOW_H
#define FLOW_H

#include "image.h"

// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// returns: velocity matrix
image optical_flow_images(image cur, image prev, int smooth, int stride);

image make_integral_image(image m);
image make_time_structure_matrix(image cur, image prev, int w);
// int stride: only calculate subset of pixels for speed
image make_velocity_image(image S, int stride);

// image m: image to draw on
// image v: the velocity image, velocity of each pixel
// float scale: scalar to multiply velocity by for drawing
void draw_flow(image* m, image v, float scale);

#endif
