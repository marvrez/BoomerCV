#include "image.h"

#include "utils.h"

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
    image out = make_image(w, h, c);
    for(int k = 0; k < c; ++k) {
        for(int i = 0; i < h; ++i) {
            for(int j = 0; j < w; ++j) {
                out.data[j + w*(i + h*k)] = (float)bytes[k + c*(j + w*i)] / 255.f;
            }
        }
    }
    return out;
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
	int start = c*m.w*m.h;
        for (int i = 0; i < m.h*m.w; ++i) {
            out.data[i] = m.data[start + i];
        }
    }
    return out;
}

image rgb_to_grayscale(image m)
{
    if(m.c == 1) return copy_image(m);

    image gray = make_image(m.w, m.h, 1);
    const float scale[] = { 0.299, 0.587, 0.114 };
    for (int k = 0; k < m.c; ++k) {
        int start = k*m.w*m.h;
        for (int i = 0; i < m.h*m.w; ++i) {
            gray.data[i] += scale[k] * m.data[start + i];
        }
    }
    return gray;
}

void rgb_to_grayscale_inplace(image* m)
{
    if(m->c != 3) return;
    const float scale[] = {0.299, 0.587, 0.114};
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            float grayscale_val = 0;
            for(int k = 0; k < 3; ++k){
                grayscale_val += scale[k] * get_pixel(*m, j, i, k);
            }
            set_pixel(m, j, i, 0, grayscale_val);
            set_pixel(m, j, i, 1, grayscale_val);
            set_pixel(m, j, i, 2, grayscale_val);
        }
    }
}

image grayscale_to_rgb(image m, float r, float g, float b)
{
    image rgb = make_empty_image(m.w, m.h, 3);
    if(m.c != 1) return rgb;

    const float scale[] = {r, g, b};
    for(int k = 0; k < 3; ++k) {
        for(int i = 0; i < m.h; ++i) {
            for(int j = 0; j < m.w; ++j) {
                rgb.data[j + m.w*(i + m.h*k)] += scale[k]*m.data[j + m.w*i];
            }
        }
    }
    return rgb;
}

void rgb_to_hsv(image* m)
{
    if (m->c != 3) return;

    float r, g, b;
    float h, s, v;
    for (int i = 0; i < m->h; ++i) {
        for (int j = 0; j < m->w; ++j) {
            r = get_pixel(*m, j, i, 0),
            g = get_pixel(*m, j, i, 1),
            b = get_pixel(*m, j, i, 2);

            float max = max3f(r, g, b), min = min3f(r, g, b);
            float delta = max - min;
            v = max;
            if (max == 0) s = h = 0; // h is actually not defined, but oh well..
            else {
                s = delta / max;

                if (r >= max) h = (g - b) / delta; // between yellow & magenta
                else if (g >= max) h = 2.f + (b - r) / delta; // between cyan & yellow
                else h = 4.f + (r - g) / delta; // between magenta & cyan

                if (h < 0.f) h += 6.f; // only positive numbers
                h /= 6.f; // normalize
            }
            set_pixel(m, j, i, 0, h);
            set_pixel(m, j, i, 1, s);
            set_pixel(m, j, i, 2, v);
        }
    }
}

void hsv_to_rgb(image* m)
{
    if(m->c != 3) return;

    float r, g, b;
    float h, s, v;
    float f, p, q, t;
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            h = 6*get_pixel(*m, j, i, 0);
            s = get_pixel(*m, j, i, 1);
            v = get_pixel(*m, j, i, 2);
            if (s == 0) {
                r = g = b = v;
            }
            else {
                int index = floor(h);
                f = h - index;
                p = v*(1 - s);
                q = v*(1 - s*f);
                t = v*(1 - s*(1 - f));
                switch (index) {
                case 0:
                    r = v; g = t; b = p;
                    break;
                case 1:
                    r = q; g = v; b = p;
                    break;
                case 2:
                    r = p; g = v; b = t;
                    break;
                case 3:
                    r = p; g = q; b = v;
                    break;
                case 4:
                    r = t; g = p; b = v;
                    break;
                default:
                    r = v; g = p; b = q;
                    break;
                }
            }
            set_pixel(m, j, i, 0, r);
            set_pixel(m, j, i, 1, g);
            set_pixel(m, j, i, 2, b);
        }
    }
}

void rgb_to_bgr(image* m)
{
    if (m->c != 3 || !m->data) return;
    for (int i = 0; i < m->w*m->h; ++i) {
        float swap = m->data[i];
        m->data[i] = m->data[i + m->w*m->h * 2];
        m->data[i + m->w*m->h * 2] = swap;
    }
}

void bgr_to_rgb(image* m)
{
    rgb_to_bgr(m);
}

void yuv_to_rgb(image* m)
{
    if(m->c != 3) return;
    float r, g, b;
    float y, u, v;
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            y = get_pixel(*m, j, i, 0);
            u = get_pixel(*m, j, i, 1);
            v = get_pixel(*m, j, i, 2);

            r = y + 1.13983f*v;
            g = y + -0.39465f*u + -0.58060f*v;
            b = y + 2.03211f*u;

            set_pixel(m, j, i, 0, r);
            set_pixel(m, j, i, 1, g);
            set_pixel(m, j, i, 2, b);
        }
    }
}

void rgb_to_yuv(image* m)
{
    if(m->c != 3) return;
    float r, g, b;
    float y, u, v;
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            r = get_pixel(*m, j, i, 0);
            g = get_pixel(*m, j, i, 1);
            b = get_pixel(*m, j, i, 2);

            y = 0.299f*r + 0.587f*g + 0.114f*b;
            u = -0.14713f*r + -0.28886f*g + 0.436f*b;
            v = 0.615f*r + -0.51499f*g + -0.10001f*b;

            set_pixel(m, j, i, 0, y);
            set_pixel(m, j, i, 1, u);
            set_pixel(m, j, i, 2, v);
        }
    }
}

void ycbcr_to_rgb(image* m)
{
    if(m->c != 3) return;
    float r, g, b;
    float y, cb, cr;
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            y = get_pixel(*m, j, i, 0);
            cb = get_pixel(*m, j, i, 1);
            cr = get_pixel(*m, j, i, 2);

            r = y + 1.402f*(cr - 0.5f);
            g = y - 0.344136f*(cb - 0.5f) - 0.714136*(cr-0.5f);
            b = y + 1.772f*(cb - 0.5f);

            set_pixel(m, j, i, 0, r);
            set_pixel(m, j, i, 1, g);
            set_pixel(m, j, i, 2, b);
        }
    }
}

void rgb_to_ycbcr(image* m)
{
    if(m->c != 3) return;
    float r, g, b;
    float y, cb, cr;
    for(int i = 0; i < m->h; ++i) {
        for(int j = 0; j < m->w; ++j) {
            r = get_pixel(*m, j, i, 0);
            g = get_pixel(*m, j, i, 1);
            b = get_pixel(*m, j, i, 2);

            y = 0.299f*r + 0.587f*g + 0.114f*b;
            cb = 0.5f - 0.168736f*r - 0.331264f*g + 0.5f*b;
            cr = 0.5f + 0.5f*r - 0.418688f*g - 0.081312f*b;

            set_pixel(m, j, i, 0, y);
            set_pixel(m, j, i, 1, cb);
            set_pixel(m, j, i, 2, cr);
        }
    }
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

static inline float bilinear_interpolate(image m, float x, float y, int c)
{
    int lx = (int) floorf(x), ly = (int) floorf(y);
    float dx = x - lx, dy = y - ly;
    float interpolated_val = get_pixel(m, x, y, c)*(1-dx)*(1-dy) +
                             get_pixel(m, x+1, y, c)*dx*(1-dy) +
                             get_pixel(m, x, y+1, c)*(1-dx)*dy +
                             get_pixel(m, x+1, y+1, c)*dx*dy;
    return interpolated_val;
}

static inline float nn_interpolate(image m, float x, float y, int c)
{
    int rounded_x = (int) round(x), rounded_y = (int) round(y);
    float interpolated_val = get_pixel(m, rounded_x, rounded_y, c);
    return interpolated_val;
}

image nn_resize(image m, int w, int h)
{
    image out = make_image(w, h, m.c);
    float w_scale = (float)m.w / w, h_scale = (float)m.h / h;
    for(int k = 0; k < m.c; ++k) {
        for(int i = 0; i < h; ++i) {
            for(int j = 0; j < w; ++j) {
                float y = (i + 0.5f)*h_scale - 0.5f;
                float x = (j + 0.5f)*w_scale - 0.5f;
                float val = nn_interpolate(m, x, y, k);

                set_pixel(&out, j, i, k, val);
            }
        }
    }
    return out;
}

image bilinear_resize(image m, int w, int h)
{
    image out = make_image(w, h, m.c);
    float w_scale = (float)m.w / w, h_scale = (float)m.h / h;
    for(int k = 0; k < m.c; ++k) {
        for(int i = 0; i < h; ++i) {
            for(int j = 0; j < w; ++j) {
                float y = (i + 0.5f)*h_scale - 0.5f;
                float x = (j + 0.5f)*w_scale - 0.5f;
                float val = bilinear_interpolate(m, x, y, k);

                set_pixel(&out, j, i, k, val);
            }
        }
    }
    return out;
}

image rotate_image(image m, float rad)
{
    int cx = m.w / 2, cy = m.h / 2;
    image rotated_image = make_image(m.w, m.h, m.c);
    for(int c = 0; c < m.c; ++c) {
        for(int y = 0; y < m.h; ++y) {
            for(int x = 0; x < m.w; ++x) {
                int rx = cosf(rad)*(x - cx) - sinf(rad)*(y - cy) + cx;
                int ry = sinf(rad)*(x - cx) + cosf(rad)*(y - cy) + cy;
                float val = bilinear_interpolate(m, rx, ry, c);

                set_pixel(&rotated_image, x, y, c, val);
            }
        }
    }
    return rotated_image;
}

image crop_image(image m, int dx, int dy, int w, int h)
{
    image cropped_image = make_image(w, h, m.c);
    for (int k = 0; k < m.c; ++k) {
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                int r = i + dy, c = j + dx;
                float val = 0;
                r = clamp(r, 0, m.h - 1);
                c = clamp(c, 0, m.w - 1);
                if (r >= 0 && r < m.h && c >= 0 && c < m.w) {
                    val = get_pixel(m, c, r, k);
                }
                set_pixel(&cropped_image, j, i, k, val);
            }
        }
    }
    return cropped_image;
}


image threshold_image(image m, float thresh)
{
    image out = make_image(m.w, m.h, m.c);
    for (int i = 0; i < m.w*m.h*m.c; ++i) {
        out.data[i] = (m.data[i] > thresh) ? 1.f : 0.f;
    }
    return out;
}

image otsu_binarize_image(image m)
{
    #define MAX_INTENSITY 256
    image out = make_image(m.w, m.h, m.c);

    int i, N = m.w*m.h;
    int hist[MAX_INTENSITY] = { 0 };
    float prob[MAX_INTENSITY], w[MAX_INTENSITY]; // pdf and cdf of intensities
    float mu[MAX_INTENSITY] = { 0.0f }; // mean value for separation
    float sigma[MAX_INTENSITY] = { 0.0f }; // inter-class variance

    // create histogram
    for (i = 0; i < N*m.c; ++i) {
        ++hist[(unsigned char)(255.f * m.data[i])];
    }
    // calculate probability density from histogram
    for (i = 0; i < MAX_INTENSITY; ++i) {
        prob[i] = (float)hist[i] / N;
    }

    w[0] = prob[0];
    for (i = 1; i < MAX_INTENSITY; ++i) {
        w[i] = w[i - 1] + prob[i];
        mu[i] = mu[i - 1] + i*prob[i];
    }

    // maximize sigma(inter-class variance) to determine optimal threshold value
    float threshold = 0.0f, max_sigma = 0.0f;
    for (i = 0; i < MAX_INTENSITY - 1; ++i) {
        if (w[i] != 0.0f && w[i] != 1.0f) {
            sigma[i] = pow(mu[MAX_INTENSITY - 1]*w[i] - mu[i], 2) / (w[i]*(1.0f - w[i]));
        }
        if (sigma[i] > max_sigma) {
            max_sigma = sigma[i];
            threshold = (float)i;
        }
    }
    threshold /= 255.f;

    // binarize based on newly found threshold
    for (i = 0; i < N*m.c; ++i) {
        out.data[i] = m.data[i] > threshold ? 1.0f : 0.0f;
    }
    #undef MAX_INTENSITY
    return out;
}

image binarize_image(image m, int reverse)
{
    image out = copy_image(m);
    for (int i = 0; i < m.w*m.h*m.c; ++i) {
        if (out.data[i] > 0.5f) out.data[i] = reverse ? 1.f : 0.f;
        else out.data[i] = reverse ? 0.f : 1.f;
    }
    return out;
}

unsigned char* get_image_data_hwc(image m)
{
    unsigned char* data = 0;
    if (m.data) {
        data = calloc(m.w*m.h*m.c, sizeof(unsigned char));
        if (data) {
            for (int k = 0; k < m.c; ++k) {
		int start = k*m.w*m.h;
                for (int i = 0; i < m.w*m.h; ++i) {
                    data[i*m.c + k] = (unsigned char)(255 * m.data[start + i]);
                }
            }
        }
    }
    return data;
}
