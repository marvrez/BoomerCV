#include "panorama.h"

#include <stdlib.h>
#include <math.h>

point make_point(float x, float y)
{
    point p = { x, y };
    return p;
}

// compute L2 distance between two points
static inline float point_distance(point p, point q)
{
    return sqrtf((p.x-q.x)*(p.x-q.x) + (p.y-q.y)*(p.y-q.y));
}

// Place two images side by side on canvas, for drawing matching pixels.
static inline image both_images(image a, image b)
{
    image both = make_image(a.w + b.w, a.h > b.h ? a.h : b.h, a.c > b.c ? a.c : b.c);
    for(int k = 0; k < a.c; ++k) {
        #pragma omp parallel for
        for(int i = 0; i < a.h; ++i) {
            for(int j = 0; j < a.w; ++j) {
                set_pixel(&both, j, i, k, get_pixel(a, j, i, k));
            }
        }
    }
    for(int k = 0; k < b.c; ++k) {
        #pragma omp parallel for
        for(int i = 0; i < b.h; ++i) {
            for(int j = 0; j < b.w; ++j) {
                set_pixel(&both, j+a.w, i, k, get_pixel(b, j, i, k));
            }
        }
    }
    return both;
}

// Calculates L1 distance between two same-size floating point arrays
static inline float l1_distance(float* a, float* b, int n)
{
    float dist = 0;
    #pragma omp parallel for reduction(+:dist)
    for(int i = 0; i < n; ++i) {
        dist += fabsf(a[i] - b[i]);
    }
    return dist;
}

// Draws lines between matching pixels in two images.
// image a, b: two images that have matches.
// match* matches: array of matches between a and b.
// int n: number of matches.
// int num_inliers: number of inliers at beginning of matches, drawn in green.
// returns: image with matches drawn between a and b on same canvas.
static inline image draw_matches(image a, image b, match* matches, int n, int num_inliers)
{
    image both = both_images(a, b);
    for(int i = 0; i < n; ++i) {
        point p = matches[i].p, q = matches[i].q;
        float dy = (float)(q.y - p.y)/(q.x+a.w - p.x);
        for(int j = p.x; j < q.x + a.w; ++j) {
            int y = dy*(j-p.x) + p.y;
            set_pixel(&both, j, y, 0, i < num_inliers ? 0.f : 1.f);
            set_pixel(&both, j, y, 1, i < num_inliers ? 1.f : 0.f);
            set_pixel(&both, j, y, 2, 0);
        }
    }
    return both;
}

// compare matches a and b
// 0 if same, 1 if a > bm, -1 if a < b
static inline int match_compare(const void* a, const void* b)
{
    match* ma = (match*)a;
    match* mb = (match*)b;
    if(ma->dist > mb->dist) return 1;
    else if(ma->dist < mb->dist) return -1;
    else return 0;
}

match* match_descriptors(descriptor* a, int an, descriptor* b, int bn, int* mn)
{
    *mn = an; // at most an matches.
    match* m = calloc(an, sizeof(match));
    #pragma omp parallel for
    for(int j = 0; j < an; ++j) {
        // for every descriptor in a, find best match in b.
        // record ai as the index in a and bi as the index in b.
        int bi = 0;
        m[j].dist = l1_distance(a[j].data, b[0].data, a[j].n);
        for(int i = 1; i < bn; ++i) {
            float dist = l1_distance(a[j].data, b[i].data, a[j].n);
            if(dist < m[j].dist) {
                m[j].dist = dist, bi = i;
            }
        }
        m[j].ai = j, m[j].bi = bi;
        m[j].p = a[j].p, m[j].q = b[bi].p;
    }

    int* seen = calloc(bn, sizeof(int)), count = 0;
    // sort matches by distance and assert that they are one-to-one
    qsort(m, an, sizeof(match), &match_compare);
    for(int i = 0; i < an; ++i) {
        if(!seen[m[i].bi]) {
            seen[m[i].bi] = 1;
            m[count++] = m[i];
        }
    }
    *mn = count;
    free(seen);
    return m;
}

image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff)
{
    return make_empty_image(1,1,1);
}

// find corners, match them and draw them between two images
image find_and_draw_matches(image a, image b, float sigma, float thresh, int nms)
{
    int num_a=0, num_b=0, num_matches=0;
    descriptor* ad = harris_corner_detector(a, sigma, thresh, nms, &num_a);
    descriptor* bd = harris_corner_detector(b, sigma, thresh, nms, &num_b);
    match* m = match_descriptors(ad, num_a, bd, num_b, &num_matches);

    draw_corners(&a, ad, num_a);
    draw_corners(&b, bd, num_b);
    image lines_image = draw_matches(a, b, m, num_matches, 0);

    free_descriptors(ad, num_a); free_descriptors(bd, num_b); free(m);
    return lines_image;
}
