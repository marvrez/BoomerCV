#ifndef DRAW_H
#define DRAW_H

#include "image.h"

void draw_box(image* m, int x1, int y1, int x2, int y2, float r, float g, float b);
void draw_circle(image* m, int x0, int y0, int radius, float r, float g, float b);
void draw_line(image* m, int x1, int y1, int x2, int y2, float r, float g, float b);
void draw_grid(image* m, float x_min, float y_min, float x_max, float y_max, int steps, float r, float g, float b);
void draw_bbox(image* m, box bbox, float r, float g, float b);
void draw_bbox_width(image* m, box bbox, int width, float r, float g, float b);
void draw_grid_width(image* m, float x_min, float y_min, float x_max, float y_max, int steps, int width, float r, float g, float b);
void draw_circle_thickness(image* m, int x0, int y0, int radius, int width, float r, float g, float b);

#endif
