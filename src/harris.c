#include "harris.h"

#include "filter.h"

#include <stdlib.h>
#include <assert.h>
#include <float.h>

// creates a descriptor for an index in an image
static inline descriptor make_descriptor(image m, int idx)
{
    const int w = 5;
    int x, y;
    descriptor d;
    x = d.p.x = idx % m.w, y = d.p.y = idx / m.w;
    d.n = w*w*m.c;
    d.data = calloc(d.n, sizeof(float));
    int count = 0;
    // subtracts the central value from neighbors to compensate some for exposure/lighting changes
    for(int c = 0; c < m.c; ++c) {
        float central_val = m.data[idx + c*m.w*m.h];
        for(int dx = -w/2; dx < (w+1)/2; ++dx) {
            for(int dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(m, x + dx, y + dy, c);
                d.data[count++] = central_val - val;
            }
        }
    }
    return d;
}

// returns S: the structure matrix for image m. 
// 1st channel is Ix^2, 2nd channel is Iy^2, third channel is IxIy.
image make_structure_matrix(image m, float sigma)
{
    int n = m.w*m.h;
    image gx_filter = make_gx_filter(), gy_filter = make_gy_filter();

    image gx, gy;
    #pragma omp parallel sections
    {
        #pragma omp section
        gx = convolve_image(m, gx_filter, 0);
        #pragma omp section
        gy = convolve_image(m, gy_filter, 0);
    }

    image D = make_image(m.w, m.h, 3);
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        int Ix = gx.data[i], Iy = gy.data[i];
        D.data[i] = Ix*Ix; 
        D.data[i + n] = Iy*Iy;
        D.data[i + 2*n] = Ix*Iy;
    }
    image S = gaussian_noise_reduce(D, sigma);

    free_image(&D);
    free_image(&gx_filter); free_image(&gy_filter);
    free_image(&gx); free_image(&gy);
    return S;
}

// image S: structure matrix for an image.
// returns R: a response map of cornerness calculations.
image harris_cornerness_response(image S)
{
    image R = make_image(S.w, S.h, 1);
    const float alpha = 0.06;
    int n = S.w*S.h;
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        float IxIx = S.data[i], IyIy = S.data[i + n];
        float IxIy = S.data[i + 2*n];

        float det = IxIx*IyIy - IxIy*IxIy;
        float trace = IxIx + IyIy;
        R.data[i] = det - alpha*trace*trace;
    }
    return R;
}

static inline void local_nms(image m, image* out, int x, int y, int w)
{
    float val = get_pixel(m, x, y, 0);
    for(int dy = -w; dy <= w; ++dy) {
        for(int dx = -w; dx <= w; ++dx) {
            float neighbor_val = get_pixel(m, x+dx, y+dy, 0);
            if(neighbor_val > val) {
                set_pixel(out, x, y, 0, -FLT_MAX);
                return;
            }
        }
    }
}

image harris_nms_image(image m, int w)
{
    image out = copy_image(m);
    #pragma omp parallel for
    for(int y = 0; y < m.h; ++y) {
        for(int x = 0; x < m.w; ++x) {
            local_nms(m, &out, x, y, w);
        }
    }
    return out;
}

descriptor* harris_corner_detector(image m, float sigma, float thresh, int nms, int* n)
{
    // calculate structure matrix
    image S = make_structure_matrix(m, sigma);
    // estimate cornerness using the structure matrix
    image R = harris_cornerness_response(S);
    // run nms on the responses
    image R_nms = harris_nms_image(R, nms);

    int count = 0;
    #pragma omp parallel for reduction(+:count)
    for(int i = 0; i < m.h*m.w; ++i){
        if(R_nms.data[i] > thresh) ++count;
    }

    *n = count;
    descriptor *d = calloc(count, sizeof(descriptor));
    int j;
    #pragma omp parallel for
    for(int i = 0, j = 0; i < m.h*m.w; ++i) {
        if(R_nms.data[i] > thresh) {
            d[j] = make_descriptor(m, i);
            #pragma omp atomic
            j++;
        }
    }
    assert(j == count);

    free_image(&S);
    free_image(&R);
    free_image(&R_nms);
    return d;
}

void draw_corners(image* m, descriptor* d, int n)
{
    #pragma omp parallel for
    for(int i = 0; i < n; ++i){
        int x = d[i].p.x, y = d[i].p.y;
        for(int j = -9; j <= 9; ++j){
            set_pixel(m, x+j, y, 0, 1);
            set_pixel(m, x, y+j, 0, 1);
            set_pixel(m, x+j, y, 1, 0);
            set_pixel(m, x, y+j, 1, 0);
            set_pixel(m, x+j, y, 2, 1);
            set_pixel(m, x, y+j, 2, 1);
        }
    }
}

void free_descriptors(descriptor* d, int n)
{
    for(int i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}
