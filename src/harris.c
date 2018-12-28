#include "harris.h"

#include "filter.h"

#include <stdlib.h>

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

image cornerness_response(image S)
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

void draw_corners(image m, descriptor* d, int n)
{
    #pragma omp parallel for
    for(int i = 0; i < n; ++i){
        int x = d[i].p.x, y = d[i].p.y;
        for(int j = -9; j <= 9; ++j){
            set_pixel(&m, x+j, y, 0, 1);
            set_pixel(&m, x, y+j, 0, 1);
            set_pixel(&m, x+j, y, 1, 0);
            set_pixel(&m, x, y+j, 1, 0);
            set_pixel(&m, x+j, y, 2, 1);
            set_pixel(&m, x, y+j, 2, 1);
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
