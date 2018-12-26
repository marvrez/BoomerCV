#include "canny.h"
#include "filter.h"

#include <stdlib.h>
#include <math.h>

#define TAN22 0.4142135623730950488016887242097f
#define TAN67 2.4142135623730950488016887242097f

#define WEAK_THRESHOLD_PERCENTAGE 0.8f // percentage of the strong threshold value that the weak threshold shall be set at
#define STRONG_THRESHOLD_PERCENTAGE 0.1f // minimum percentage of pixels that are considered to meet the strong threshold

#define MAX_INTENSITY 256

image canny_image(image m, int reduce_noise)
{
    image out, sobel, clean;
    int weak_threshold, strong_threshold, *G, *theta;
    float sigma = 1.4f;

    if(!m.data || m.c == 1) return make_empty_image(0,0,0);

    clean = reduce_noise ? gaussian_noise_reduce(m, sigma) : copy_image(m);

    sobel = make_image(m.w, m.h, 1);
    out = make_image(m.w, m.h, 1);
    G = malloc(m.w*m.h*sizeof(int));
    theta = malloc(m.w*m.h*sizeof(int));

    canny_sobel_image(clean, G, theta);
    canny_nms(G, theta, &sobel);
    canny_estimate_threshold(sobel, &weak_threshold, &strong_threshold);
    canny_hysteresis(weak_threshold, strong_threshold, sobel, &out);

    free(G);
    free(theta);
    free_image(&clean);
    free_image(&sobel);
    return out;
}

void canny_sobel_image(image in, int* G, int* theta)
{
    float g_div;
    int w = in.w, h = in.h;
    #pragma omp parallel for
    for(int y = w * 3; y < w*(h - 3); y += w) {
        for(int x = 3; x < w - 3; ++x) {
            int g_x = (int)(255*(2*in.data[x + y + 1]
                + in.data[x + y - w + 1]
                + in.data[x + y + w + 1]
                - 2*in.data[x + y - 1]
                - in.data[x + y - w - 1]
                - in.data[x + y + w - 1]));
            int g_y = (int)(255*(2*in.data[x + y - w]
                + in.data[x + y - w + 1]
                + in.data[x + y - w - 1]
                - 2*in.data[x + y + w]
                - in.data[x + y + w + 1]
                - in.data[x + y + w - 1]));

            G[x + y] = sqrt(g_x*g_x + g_y*g_y);
            // '|' = 0, '\' = 1, '-' = 2, '/' = 3
            if (g_x == 0) theta[x + y] = 2;
            else {
                g_div = g_y / (float)g_x;
                if (g_div < 0) {
                    if (g_div < -TAN67) theta[x + y] = 0;
                    else {
                        if (g_div < -TAN22) theta[x + y] = 1;
                        else theta[x + y] = 2;
                    }
                }
                else {
                    if (g_div > TAN67) theta[x + y] = 0;
                    else {
                        if (g_div > TAN22) theta[x + y] = 3;
                        else theta[x + y] = 2;
                    }
                }
            }
        }
    }
}

void canny_nms(int* G, int* theta, image* out)
{
    int w = out->w, h = out->h;
    #pragma omp parallel for
    for(int y = 0; y < w*h; y += w) {
        for(int x = 0; x < w; ++x) {
            switch(theta[x + y]) {
                case 0: // '|'
                    if(G[x + y] > G[x + y - w] && G[x + y] > G[x + y + w])
                        out->data[x+y] = G[x+y] > 255 ?  255.f : G[x+y];
                    else out->data[x + y] = 0.f;
                    break;
                case 1: // '\'
                    if (G[x + y] > G[x + y - w - 1] && G[x + y] > G[x + y + w + 1])
                        out->data[x+y] = G[x+y] > 255 ?  255.f : G[x+y];
                    else out->data[x + y] = 0.f;
                    break;
                case 2: // '-'
                    if (G[x + y] > G[x + y - 1] && G[x + y] > G[x + y + 1])
                        out->data[x+y] = G[x+y] > 255 ?  255.f : G[x+y];
                    else out->data[x + y] = 0.f;
                    break;
                case 3: // '/'
                    if (G[x + y] > G[x + y - w + 1] && G[x + y] > G[x + y + w - 1])
                        out->data[x+y] = G[x+y] > 255 ?  255.f : G[x+y];
                    else out->data[x + y] = 0.f;
                    break;
                default:
                    break;
            }
        }
    }
}

// - heuristic for estimating a double threshold -
// assumes that the top x% (given by STRONG_THRESHOLD_PERCENTAGE) of edge pixels with the highest intensity are the true edges 
// and that the weak threshold is equal to the quantity of strong_threshold plus the total number of 0s at the low end of the histogram
void canny_estimate_threshold(image m, int* weak_threshold, int* strong_threshold)
{
    int i, n = m.w*m.h, strong_cutoff = 0, hist[MAX_INTENSITY] = {0};
    #pragma omp parallel for
    for (i = 0; i < n; ++i) ++hist[(int)m.data[i]];
    int pixels = (n - hist[0])*STRONG_THRESHOLD_PERCENTAGE;

    i = MAX_INTENSITY - 1;
    while(strong_cutoff < pixels) strong_cutoff += hist[i--];
    *strong_threshold = i;

    i = 1;
    while(hist[i] == 0) i++;
    *weak_threshold = (*strong_threshold + i)*WEAK_THRESHOLD_PERCENTAGE;
}

static inline int canny_in_range(image m, int x, int y)
{
    return x >= 0 && x < m.w && y >= 0 && y < m.h;
}
