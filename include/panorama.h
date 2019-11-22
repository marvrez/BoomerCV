#ifndef PANORAMA_H
#define PANORAMA_H

#include "image.h"

#include "matrix.h"
#include "harris.h"

// A match between two points in an image.
// point p, q: x,y coordinates of the two matching pixels.
// int ai, bi: indexes in the descriptor array. For eliminating duplicates.
// float dist: the distance between the descriptors for the points.
typedef struct {
    point p, q;
    int ai, bi;
    float dist;
} match;

// projection functions
point make_point(float x, float y);
point project_point(matrix H, point p);
matrix compute_homography(match* matches, int n);
image cylindrical_project(image m, float f);

// stitching functions
int model_inliers(matrix H, match* m, int n, float thresh);
image combine_images(image a, image b, matrix H);
match* match_descriptors(descriptor* a, int an, descriptor* b, int bn, int* mn);
image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff);

image find_and_draw_matches(image a, image b, float sigma, float thresh, int nms);

#endif
