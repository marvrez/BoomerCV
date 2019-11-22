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
        #pragma omp parallel for
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

static inline void transpose_1d_filter(image* filter)
{
    assert(filter->w == 1 || filter->h == 1);
    int tmp = filter->w;
    filter->w = filter->h;
    filter->h = tmp;
}

image make_gx_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = 0; f.data[2] = 1;
    f.data[3] = -2; f.data[4] = 0; f.data[5] = 2;
    f.data[6] = -1; f.data[7] = 0; f.data[8] = 1;
    return f;
}

image make_gy_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = -2; f.data[2] = -1;
    f.data[3] =  0; f.data[4] =  0; f.data[5] =  0;
    f.data[6] =  1; f.data[7] =  2; f.data[8] =  1;
    return f;
}

static inline image make_sharpen_filter()
{
    image f = make_image(3,3,1);
    f.data[0] =  0; f.data[1] = -1; f.data[2] =  0;
    f.data[3] = -1; f.data[4] =  5; f.data[5] = -1;
    f.data[6] =  0; f.data[7] = -1; f.data[8] =  0;
    return f;
}

static inline image make_1d_box(int w)
{
    image f = make_image(1, w, 1);
    for (int i = 0; i < w; ++i) {
        f.data[i] = 1.f / w;
    }
    return f;
}

static inline image make_1d_gaussian(float sigma)
{
    int w = ((int)(3*sigma)) | 1;
    int offset = w / 2;
    image f = make_image(1, w, 1);
    for(int i = 0 - offset; i < w - offset; ++i) {
        float val = 1.f/sqrtf(2*M_PI*sigma*sigma)*expf((-i*i)/(2.f*sigma*sigma));
        set_pixel(&f, 0, i + offset, 0, val);
    }
    return f;
}

image equalize_histogram(image m)
{
    #define MAX_INTENSITY 256
    image out;
    int n = m.w*m.h, count = 0, hist[MAX_INTENSITY] = {0};
    float transform_table[MAX_INTENSITY] = {0.f}, cdf[MAX_INTENSITY] = {0.f};

    if(m.c == 3) {
        rgb_to_ycbcr(&m);
        out = copy_image(m);
    }
    else out = make_image(m.w, m.h, m.c);

    // Generate histogram
    #pragma omp parallel for reduction(+:hist)
    for(int i = 0; i < n; ++i) {
        ++hist[(unsigned char)(255*m.data[i])];
    }
    // Generation of transform table
    for(int i = 0; i < MAX_INTENSITY; ++i) {
        count += hist[i];
        cdf[i] = 1.f*count/n;
        transform_table[i] = floorf(cdf[i]*(MAX_INTENSITY-1))/255.f;
    }
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        out.data[i] = transform_table[(unsigned char)(255*m.data[i])];
    }
    normalize_image(&out);
    if(m.c == 3) {
        ycbcr_to_rgb(&out);
        clamp_image(&out);
        ycbcr_to_rgb(&m);
    }
    #undef MAX_INTENSITY
    return out;
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
    #pragma omp parallel for
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

image smoothen_image(image m, int w)
{
    image box_1d = make_1d_box(w);
    image out_tmp = convolve_image(m, box_1d, 1);
    transpose_1d_filter(&box_1d);
    image out = convolve_image(out_tmp, box_1d, 1);
    free_image(&box_1d); free_image(&out_tmp);
    return out;
}

// helper function for canny edge detection
image gaussian_noise_reduce(image m, float sigma)
{
#if 0
    image gauss_1d = make_1d_gaussian(sigma);
    image out_tmp = convolve_image(m, gauss_1d, 1);
    transpose_1d_filter(&gauss_1d);
    image out = convolve_image(out_tmp, gauss_1d, 1);
    free_image(&gauss_1d); free_image(&out_tmp);
    return out;
#else
    // implementation based on http://blog.ivank.net/fastest-gaussian-blur.html
    image out = make_image(m.w, m.h, m.c);
    int w = ((int)(sqrtf(3*sigma*sigma+1))) | 1;
    int r = (w - 1)/2;
    float gamma = 1.f / (r+r+1);
    #pragma omp parallel for
    for(int k = 0; k < m.c; ++k) {
        // blur horizontally for each row
        const int idx = m.w*m.h*k;
        const float* scl = m.data + idx; float* tcl = out.data + idx;
        for(int i = 0; i < m.h; ++i) {
            int ti = i*m.w, li = ti, ri = ti+r;
            float fv = scl[ti], lv = scl[ti+m.w-1], val = 0;
            #pragma omp simd reduction(+:val)
            for(int j=0; j<r; ++j) val += scl[ti+j];
            val += (r+1)*fv;
            for(int j=0; j<=r; ++j) { val += scl[ri++] - fv; tcl[ti++] = val*gamma; }
            for(int j=r+1; j<m.w-r; ++j) { val += scl[ri++] - scl[li++]; tcl[ti++] = val*gamma; }
            for(int j=m.w-r; j<m.w; ++j) { val += lv - scl[li++]; tcl[ti++] = val*gamma; }
        }
        // blur vertically for each column
        scl = out.data + idx; tcl = out.data + idx;
        for(int i = 0; i < m.w; ++i) {
            int ti = i, li = ti, ri = ti+r*m.w;
            float fv = scl[ti], lv = scl[ti+m.w*(m.h-1)], val = 0;
            #pragma omp simd reduction(+:val)
            for(int j=0; j<r;  ++j) val += scl[ti + j*m.w];
            val += (r+1)*fv;
            for(int j=0; j<=r; ++j) { val += scl[ri] - fv; tcl[ti] = val*gamma; ri+=m.w; ti+=m.w; }
            for(int j=r+1; j<m.h-r; ++j) { val += scl[ri] - scl[li];  tcl[ti] = val*gamma; li+=m.w; ri+=m.w; ti+=m.w; }
            for(int j=m.h-r; j<m.h; ++j) { val += lv - scl[li]; tcl[ti] = val*gamma; li+=m.w; ti+=m.w; }
        }
    }
    return out;
#endif
}

image dilate_image(image m, int times)
{
    if(m.c != 1) return make_empty_image(0,0,0);

    image out = make_image(m.w, m.h, m.c);
    image tmp = copy_image(m);
    while(times--) {
        #pragma omp parallel for
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
        #pragma omp parallel for
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

static inline int hilditch_func_nc8(int *b)
{
    int n_odd[4] = { 1, 3, 5, 7 }; // odd-number neighbors
    int j, sum = 0, d[10];

    for(int i = 0; i <= 9; ++i) {
        d[i] = abs(b[i%8]) == 1 ? 1 : 0;
    }
    for(int i = 0; i < 4; ++i) {
        j = n_odd[i];
        sum += d[j] - d[j]*d[j + 1]*d[j + 2];
    }
    return sum;
}

image skeletonize_image(image m)
{
    // hilditch's algorithm
    int offset[9][2] = { { 0,0 },{ 1,0 },{ 1,-1 },{ 0,-1 },{ -1,-1 },
                         { -1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };
    int n_odd[4] = { 1, 3, 5, 7 }; // odd-number neighbors
    int b[9]; // gray levels for the 9 neighbors
    int condition[6] = {0}; // valid for conditions 1-6
    int counter; // number of changing points
    int i, sum;
    image out;

    if(!m.data || m.c != 1) return make_empty_image(0, 0, 0);
    out = copy_image(m);
    do {
        counter = 0;
        for(int y = 0; y < m.h; ++y) {
            for(int x = 0; x < m.w; ++x) {
                // substitution of 9-neighbor gray values
                for(i = 0; i < 9; ++i) {
                    b[i] = 0;
                    int nx = x + offset[i][0], ny = y + offset[i][1];
                    if (nx >= 0 && nx < m.w && ny >= 0 && ny < m.h) {
                        if(out.data[ny*m.w + nx] == 0) b[i] = 1;
                        else if(out.data[ny*m.w + nx] == 2) b[i] = -1;
                    }
                }
                for(i = 0; i < 6; ++i) condition[i] = 0;
                // condition 1: figure point
                if(b[0] == 1) condition[0] = 1;

                // condition 2: boundary point
                sum = 0;
                for(i = 0; i < 4; ++i) {
                    sum += 1 - abs(b[n_odd[i]]);
                }
                if(sum >= 1) condition[1] = 1;

                // condition 3: endpoint conservation
                sum = 0;
                for(i = 1; i <= 8; ++i) {
                    sum += abs(b[i]);
                }
                if (sum >= 2) condition[2] = 1;

                // condition 4: isolated point conservation
                sum = 0;
                for (i = 1; i <= 8; ++i) {
                    if (b[i] == 1) ++sum;
                }
                if (sum >= 1) condition[3] = 1;

                // condition 5: connectivity conservation
                if (hilditch_func_nc8(b) == 1) condition[4] = 1;

                // condition 6: one-side elimination for line-width of two
                sum = 0;
                for(i = 1; i <= 8; ++i) {
                    if (b[i] != -1) ++sum;
                    else {
                        int tmp = b[i];
                        b[i] = 0;
                        if (hilditch_func_nc8(b) == 1) ++sum;
                        b[i] = tmp;
                    }
                }
                if(sum == 8) condition[5] = 1;

                // final decision
                if(condition[0] && condition[1] && condition[2] &&
                condition[3] && condition[4] && condition[5]) {
                    out.data[y*m.w + x] = 2;
                    ++counter;
                }
            } // end of x
        } // end of y

        if(counter != 0) {
            #pragma omp parallel for
            for(i = 0; i < m.w*m.h; ++i) {
                if(out.data[i] == 2) out.data[i] = 1;
            }
        }
    } while(counter != 0);

    return out;
}
