#ifndef HOUGH_H
#define HOUGH_H

#include "image.h"

typedef struct {
    int w, h;
    unsigned int* histogram;
} accumulator;

accumulator hough_transform(image m);
line* hough_line_detect(image m, int threshold, int* num_lines);

void draw_hough_lines(image* m, line* lines, int num_lines, float r, float g, float b);

#endif
