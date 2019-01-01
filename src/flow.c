#include "flow.h"

#include "filter.h"

#include <math.h>

static inline void constrain_image(image* m, float v)
{
    #pragma omp parallel for
    for(int i = 0; i < m->w*m->h*m->c; ++i) {
        if(m->data[i] < -v) m->data[i] = -v;
        if(m->data[i] >  v) m->data[i] =  v;
    }
}

static inline void flow_draw_line(image* m, float x, float y, float dx, float dy)
{
    float angle = 6.f*(atan2(dy, dx) / (2*M_PI) + 0.5f);
    int index = floor(angle);
    float f = angle - index;
    float r, g, b;
    switch(index) {
        case 0:
            r = 1; g = f; b = 0;
            break;
        case 1:
            r = 1-f; g = 1; b = 0;
            break;
        case 2:
            r = 0; g = 1; b = f;
            break;
        case 3:
            r = 0; g = 1-f; b = 1;
            break;
        case 4:
            r = f; g = 0; b = 1;
            break;
        default:
            r = 1; g = 0; b = 1-f;
            break;
    }
    float d = sqrtf(dx*dx + dy*dy);
    for(float i = 0; i < d; i += 1) {
        int xi = x + dx*i/d, yi = y + dy*i/d;
        set_pixel(m, xi, yi, 0, r);
        set_pixel(m, xi, yi, 1, g);
        set_pixel(m, xi, yi, 2, b);
    }
}

image make_integral_image(image m)
{
    int n = m.w*m.h;
    image integ = make_image(m.w, m.h, m.c);
    #pragma omp parallel for
    for (int z = 0; z < m.c; ++z) {
        for (int y = 0; y < m.h; ++y) {
            for (int x = 0; x < m.w; ++x) {
                float v = m.data[x + y*m.w + z*n];
                if (y > 0 && x > 0) v -= integ.data[(x-1) + (y-1)*m.w + z*n];
                if (y > 0) v += integ.data[x + (y-1)*m.w + z*n];
                if (x > 0) v += integ.data[(x-1) + y*m.w + z*n];
                integ.data[x+y*m.w+z*n] = v;
            }
        }
    }
    return integ;
}

image flow_smooth_image(image m, int w)
{
    image integ = make_integral_image(m);
    image S = make_image(m.w, m.h, m.c);
    const int offset = w / 2, n = m.w*m.h;
    const float scale_factor = 1.f / (w*w);
    for(int k = 0; k < m.c; ++k) {
        int start_pos = k*n;
        #pragma omp parallel for
        for(int y = 0; y < m.h; ++y) {
            for(int x = 0; x < m.w; ++x) {
                float val = integ.data[start_pos + x - offset - 1 + m.w*(y - offset -1)] -
                            integ.data[start_pos + x + offset + m.w*(y - offset - 1)] -
                            integ.data[start_pos + x - offset - 1 + m.w*(y + offset)] +
                            integ.data[start_pos + x + offset + m.w*(y + offset)];
                S.data[start_pos + x + y*m.w] = val*scale_factor;
            }
        }
    }
    free_image(&integ);
    return S;
}

image optical_flow_images(image cur, image prev, int smooth, int stride)
{
    image S = make_time_structure_matrix(cur, prev, smooth);
    image v = make_velocity_image(S, stride);
    constrain_image(&v, 6);
    image smoothed_velocity = flow_smooth_image(v, 2);
    free_image(&v); free_image(&S);
    return smoothed_velocity;
}

image make_time_structure_matrix(image cur, image prev, int w)
{
    int n = cur.w*cur.h;
    int converted = 0;
    if(cur.c == 3) {
        converted = 1;
        #pragma omp parallel sections
        {
            #pragma omp section
            cur = rgb_to_grayscale(cur);
            #pragma omp section
            prev = rgb_to_grayscale(prev);
        }
    }

    image T = make_image(cur.w, cur.h, 5);
    image gx_filter = make_gx_filter(), gy_filter = make_gy_filter();

    image Ix, Iy, It;
    #pragma omp parallel sections
    {
        #pragma omp section
        Ix = convolve_image(cur, gx_filter, 0);
        #pragma omp section
        Iy = convolve_image(cur, gy_filter, 0);
        #pragma omp section
        It = make_image(cur.w, cur.h, 1);
    }

    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        It.data[i] = cur.data[i] - prev.data[i];
    }

    #pragma omp parallel for
    for(int i = 0;i < n; ++i) {
        T.data[i]     = Ix.data[i]*Ix.data[i];
        T.data[i+n]   = Iy.data[i]*Iy.data[i];
        T.data[i+2*n] = Ix.data[i]*Iy.data[i];
        T.data[i+3*n] = Ix.data[i]*It.data[i];
        T.data[i+4*n] = Iy.data[i]*It.data[i];
    }
    image S = flow_smooth_image(T, w);

    if(converted) {
        free_image(&cur); free_image(&prev);
    }
    free_image(&T);
    free_image(&gx_filter); free_image(&gy_filter);
    free_image(&Ix); free_image(&Iy); free_image(&It);

    return S;
}

image make_velocity_image(image S, int stride)
{
}

void draw_flow(image* m, image v, float scale)
{
    int stride = m->w / v.w;
    float stride_reciprocal = 1.f / stride;
    for(int y = (stride-1)/2; y < m->h; y += stride) {
        for(int x = (stride-1)/2; x < m->w; x += stride) {
            float dx = scale*get_pixel(v, x*stride_reciprocal, y*stride_reciprocal, 0);
            float dy = scale*get_pixel(v, x*stride_reciprocal, y*stride_reciprocal, 1);
            if(fabs(dx) > m->w) dx = 0;
            if(fabs(dy) > m->h) dy = 0;
            flow_draw_line(m, x, y, dx, dy);
        }
    }
}
