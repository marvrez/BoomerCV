#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

image make_image(int w, int h, int c)
{
    image m = make_empty_image(w, h, c);
    m.data = calloc(w*h*c, sizeof(float));
    return m;
}

image make_empty_image(int w, int h, int c)
{
    image m;
    m.data = NULL;
    m.h = h;
    m.w = w;
    m.c = c;
    return m;
}

image make_image_from_hwc_bytes(int w, int h, int c, unsigned char* bytes)
{
    image m = make_image(w, h, c);
    for(int k = 0; k < c; ++k) {
        for(int j = 0; j < h; ++j) {
            for(int i = 0; i < w; ++i) {
                m.data[i + w*j + w*h*k] = (float)bytes[k + c*i + c*w*j]/255.f;
            }
        }
    }
    return m;
}

image copy_image(image m) 
{
    image copy = m;
    copy.data = calloc(m.w*m.h*m.c, sizeof(float));
    if (copy.data && m.data) {
        memcpy(copy.data, m.data, m.w*m.h*m.c*sizeof(float));
    }
    return copy;
}

void free_image(image* m)
{
    if (m->data) {
        free(m->data);
    }
}

image load_image(const char* filename, int num_channels)
{
    int w, h, c;
    unsigned char* data = stbi_load(filename, &w, &h, &c, num_channels);
    if (!data) {
        fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n", filename, stbi_failure_reason());
        exit(0);
    }
    if(num_channels) c = num_channels;
    image m = make_image_from_hwc_bytes(w,h,c,data);
    free(data);
    return m;
}

image load_image_from_memory(const unsigned char* buffer, int buf_len, int num_channels)
{
    int w, h, c;
    unsigned char* data = stbi_load_from_memory(buffer, buf_len, &w, &h, &c, num_channels);
    if (!data) {
        fprintf(stderr, "Cannot load image from memory.\nSTB Reason: %s\n", stbi_failure_reason());
        exit(0);
    }
    if(num_channels) c = num_channels;
    image m = make_image_from_hwc_bytes(w,h,c,data);
    free(data);
    return m;
}

image load_image_rgb(const char* filename)
{
    image out = load_image(filename, 3);
    return out;
}

image load_image_grayscale(const char* filename)
{
    image out = load_image(filename, 1);
    return out;
}

int save_image_png(image m, const char* filename)
{
    char buffer[256];
    sprintf(buffer, "%s.png", filename);
    unsigned char* pixels = get_image_data_hwc(m);
    int success = stbi_write_png(buffer, m.w, m.h, m.c, pixels, m.w*m.c);
    if(!success) fprintf(stderr, "Failed to write image %s\n", buffer);
    return success;
}

int save_image_jpg(image m, const char* filename, int quality)
{
    char buffer[256];
    sprintf(buffer, "%s.jpg", filename);
    unsigned char* pixels = get_image_data_hwc(m);
    int success = stbi_write_jpg(buffer, m.w, m.h, m.c, pixels, quality);
    if(!success) fprintf(stderr, "Failed to write image %s\n", buffer);
    return success;
}

image get_channel(image m, int c)
{
    image out = make_image(m.w, m.h, 1);
    if (out.data && c >= 0 && c < m.c) {
        for (int i = 0; i < m.h*m.w; ++i) {
            out.data[i] = m.data[i + c*m.w*m.h];
        }
    }
    return out;
}

void fill_image(image* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i) {
        m->data[i] = s;
    }
}

void clamp_image(image* m)
{
    for(int i = 0; i < m->w*m->h*m->c; ++i) {
        if(m->data[i] < 0.f) m->data[i] = 0.f;
        if(m->data[i] > 1.f) m->data[i] = 1.f;
    }
}

void translate_image(image* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i) {
        m->data[i] += s;
    }
}

void scale_image(image* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i) {
        m->data[i] *= s;
    }
}

void normalize_image(image* m)
{
    float min = FLT_MAX, max = -FLT_MAX;
    for(int i = 0; i < m->w*m->h*m->c; ++i) {
        float val = m->data[i];
        if(val < min) min = val;
        if(val > max) max = val;
    }

    if(fabsf(max - min) < 1e-6) {
        min = 0;
        max = 1;
    }

    for(int i = 0; i < m->w*m->h*m->c; ++i) {
        m->data[i] = (m->data[i] - min) / (max - min);
    }
}

void transpose_image(image* m)
{
    if(m->w != m->h) {
        fprintf(stderr, "image did not get transposed as width=%d and height=%d are not the same.", m->w, m->h);
        return;
    }
    for(int k = 0; k < m->c; ++k) {
        for(int i = 0; i < m->w-1; ++i) {
            for(int j = i + 1; j < m->w; ++j) {
                int idx = j + m->w*(i + m->h*k);
                int transposed_idx = i + m->w*(j + m->h*k);

                float tmp = m->data[idx];
                m->data[idx] = m->data[transposed_idx];
                m->data[transposed_idx] = tmp;
            }
        }
    }
}

void flip_image(image* m)
{
    for(int k = 0; k < m->c; ++k) {
        for(int i = 0; i < m->h; ++i) {
            for(int j = 0; j < m->w; ++j) {
                int idx = j + m->w*(i + m->h*k);
                int flip_idx = (m->w - j - 1) + m->w*(i + m->h*k);

                float tmp  = m->data[idx];
                m->data[idx] = m->data[flip_idx];
                m->data[flip_idx] = tmp;
            }
        }
    }
}

unsigned char* get_image_data_hwc(image m)
{
    unsigned char* data = 0;
    if (m.data) {
        data = calloc(m.w*m.h*m.c, sizeof(unsigned char));
        if (data) {
            for (int k = 0; k < m.c; ++k) {
                for (int i = 0; i < m.w*m.h; ++i) {
                    data[i*m.c + k] = (unsigned char)(255 * m.data[i + k*m.w*m.h]);
                }
            }
        }
    }
    return data;
}
