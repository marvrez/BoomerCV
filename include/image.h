#ifndef IMAGE_H
#define IMAGE_H

#include "utils.h"

typedef struct {
    int c, h, w;
    float* data;
} image;

typedef struct {
    int x, y;
    int w, h;
    float score;
    const char name;
} box;

typedef struct {
    float x, y;
} point;

image make_image(int w, int h, int c);
image make_empty_image(int w, int h, int c);
image make_image_from_hwc_bytes(int w, int h, int c, unsigned char* bytes);
image copy_image(image m);
void free_image(image* m);

float get_pixel(image m, int x, int y, int c);
void set_pixel(image* m, int x, int y, int c, float v);

// load functions
image load_image(const char* filename, int num_channels);
image load_image_from_memory(const unsigned char* buffer, int buf_len, int num_channels);
image load_image_rgb(const char* filename);
image load_image_grayscale(const char* filename);

// save functions
int save_image_png(image m, const char* filename);
int save_image_jpg(image m, const char* filename, int quality);

image get_channel(image m, int c);

// colorspace functions
void rgb_to_hsv(image* m);
void hsv_to_rgb(image* m);
void rgb_to_bgr(image* m);
void bgr_to_rgb(image* m);
void yuv_to_rgb(image* m);
void rgb_to_yuv(image* m);
image rgb_to_grayscale(image m);
image grayscale_to_rgb(image m, float r, float g, float b);

// image operations
void fill_image(image* m, float s);
void clamp_image(image* m);
void translate_image(image* m, float s);
void scale_image(image* m, float s);
void normalize_image(image* m);
void transpose_image(image* m);
void flip_image(image* m);

image rotate_image(image m, float rad);
image crop_image(image m, int dx, int dy, int w, int h);

// resizing
float nn_interpolate(image m, float x, float y, int c);
image nn_resize(image m, int w, int h);
float bilinear_interpolate(image m, float x, float y, int c);
image bilinear_resize(image m, int w, int h);

unsigned char* get_image_data_hwc(image m);

#endif
