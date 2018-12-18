#include "filter.h"

#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

image convolve_image(image m, image filter, int preserve)
{
    assert(m.c == filter.c || filter.c == 1);
    int single_channel = filter.c == 1;
    int k, i, j, dx, dy;
    image out = make_image(m.w, m.h, preserve ? m.c : 1);
    for(k = 0; k < m.c; ++k) {
        int filter_channel = single_channel ? 0 : k, out_channel = preserve ? k : 0;
        for(i = 0; i < m.h; ++i) {
            for(j = 0; j < m.w; ++j) {
                float sum = 0.f;
                for(dy = 0; dy < filter.h; ++dy) {
                    for(dx = 0; dx < filter.w; ++dx) {
                        sum += get_pixel(filter, dx, dy, filter_channel) *
                                get_pixel(m, j-filter.w/2+dx, i-filter.h/2+dy, k);
                    }
                }
                out.data[j + out.w*(i + out_channel*out.h)] += sum;
            }
        }
    }
    return out;
}

inline image make_gx_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = 0; f.data[2] = 1;
    f.data[3] = -2; f.data[4] = 0; f.data[5] = 2;
    f.data[6] = -1; f.data[7] = 0; f.data[8] = 1;
    return f;
}

inline image make_gy_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = -2; f.data[2] = -1;
    f.data[3] =  0; f.data[4] =  0; f.data[5] =  0;
    f.data[6] =  1; f.data[7] =  2; f.data[8] =  1;
    return f;
}

inline image make_sharpen_filter()
{
    image f = make_image(3,3,1);
    f.data[0] =  0; f.data[1] = -1; f.data[2] =  0;
    f.data[3] = -1; f.data[4] =  5; f.data[5] = -1;
    f.data[6] =  0; f.data[7] = -1; f.data[8] =  0;
    return f;
}

inline image make_smoothing_filter()
{
    image f = make_image(3,3,1);
    float val = 1.f / 9.f;
    f.data[0] = val; f.data[1] = val; f.data[2] = val;
    f.data[3] = val; f.data[4] = val; f.data[5] = val;
    f.data[6] = val; f.data[7] = val; f.data[8] = val;
    return f;
}

image colorize_sobel(image m)
{
    image* s = sobel_image(m);
    normalize_image(&s[0]); normalize_image(&s[1]);
    image out = make_image(m.w, m.h, 3);
    memcpy(out.data, s[1].data, m.w*m.h*sizeof(float));
    memcpy(out.data+m.w*m.h, s[0].data, m.w*m.h*sizeof(float));
    memcpy(out.data+2*m.w*m.h, s[0].data, m.w*m.h*sizeof(float));
    hsv_to_rgb(&out);
    free_image(&s[0]); free_image(&s[1]);
    free(s);
    return out;
}

image* sobel_image(image m)
{
    image* out = calloc(2, sizeof(image));
    image G = make_image(m.w, m.h, 1), theta = make_image(m.w, m.h, 1);
    out[0] = G, out[1] = theta;
    image gx_filter = make_gx_filter(), gy_filter = make_gy_filter();
    image Gx = convolve_image(m, gx_filter, 0), Gy = convolve_image(m, gy_filter, 0);
    for(int i = 0; i < m.w*m.h; ++i) {
        G.data[i] = sqrtf(Gx.data[i]*Gx.data[i] + Gy.data[i]*Gy.data[i]);
        theta.data[i] = atan2(Gy.data[i], Gx.data[i]);
    }
    free_image(&gx_filter); free_image(&Gx);
    free_image(&gy_filter); free_image(&Gy);
    return out;
}

image sharpen_image(image m)
{
    image sharpen_filter = make_sharpen_filter();
    image out = convolve_image(m, sharpen_filter, 1);
    clamp_image(&out);
    free_image(&sharpen_filter);
    return out;
}

image smoothen_image(image m)
{
    image smoothing_filter = make_smoothing_filter();
    image out = convolve_image(m, smoothing_filter, 1);
    clamp_image(&out);
    free_image(&smoothing_filter);
    return out;
}

image dilate_image(image m, int times)
{
    if(m.c != 1) return make_empty_image(0,0,0);

    image out = make_image(m.w, m.h, m.c);
    image tmp = copy_image(m);
    while(times--) {
        for(int y = 0; y < m.h; ++y) {
            for(int x = 0; x < m.w; ++x) {
                int x2, y2, x3, y3;
                y2 = (y - 1 < 0) ? m.h - 1 : y - 1;
                x2 = (x - 1 < 0) ? m.w - 1 : x - 1;
                y3 = (y + 1 >= m.h) ? 0 : y + 1;
                x3 = (x + 1 >= m.w) ? 0 : x + 1;

                float t = tmp.data[y*m.w + x];
                if(tmp.data[y2*m.w + x] > t) t = tmp.data[y2*m.w + x];
                if(tmp.data[y3*m.w + x] > t) t = tmp.data[y3*m.w + x];
                if(tmp.data[y*m.w + x2] > t) t = tmp.data[y*m.w + x2];
                if(tmp.data[y*m.w + x3] > t) t = tmp.data[y*m.w + x3];
                out.data[y*m.w + x] = t;
            }
        }
        memcpy(tmp.data, out.data, m.w*m.h*sizeof(float));
    }
    free_image(&tmp);
    return out;
}

image erode_image(image m, int times)
{
    if (m.c != 1) return make_empty_image(0,0,0);

    image out = make_image(m.w, m.h, m.c);
    image tmp = copy_image(m);
    while(times--) {
        for(int y = 0; y < m.h; ++y) {
            for(int x = 0; x < m.w; ++x) {
                int x2, y2, x3, y3;
                y2 = (y - 1 < 0) ? m.h - 1 : y - 1;
                x2 = (x - 1 < 0) ? m.w - 1 : x - 1;
                y3 = (y + 1 >= m.h) ? 0 : y + 1;
                x3 = (x + 1 >= m.w) ? 0 : x + 1;

                float t = tmp.data[y*m.w + x];
                if(tmp.data[y2*m.w + x] < t) t = tmp.data[y2*m.w + x];
                if(tmp.data[y3*m.w + x] < t) t = tmp.data[y3*m.w + x];
                if(tmp.data[y*m.w + x2] < t) t = tmp.data[y*m.w + x2];
                if(tmp.data[y*m.w + x3] < t) t = tmp.data[y*m.w + x3];
                out.data[y*m.w + x] = t;
            }
        }
        memcpy(tmp.data, out.data, m.w*m.h*sizeof(float));
    }
    free_image(&tmp);
    return out;
}
