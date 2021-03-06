#include "panorama.h"

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

static inline point make_point(float x, float y)
{
    point p = {x, y};
    return p;
}

static inline matrix make_homogenous(point p)
{
    matrix h = make_matrix(3, 1);
    h.data[0][0] = p.x, h.data[1][0] = p.y, h.data[2][0] = 1.f;
    return h;
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

static inline void shuffle_matches(match* m, int n)
{
    for(int i = n - 1; i > 0; --i){
        int j = rand() % (i + 1);
        match tmp = m[i];
        m[i] = m[j];
        m[j] = tmp;
    }
}

// Apply a projective transformation to a point.
// matrix H: homography to project point.
// point p: point to project.
point project_point(matrix H, point p)
{
    matrix c = make_homogenous(p);
    matrix x = multiply_matrix(H, c);
    point q = { x.data[0][0] / x.data[2][0], x.data[1][0] / x.data[2][0] };
    free_matrix(&c); free_matrix(&x);
    return q;
}

// Computes homography between two images given matching pixels.
// returns: matrix representing homography H that maps image a to image b.
matrix compute_homography(match* matches, int n)
{
    // estimate projection parameters
    matrix M = make_matrix(n*2, 8), b = make_matrix(n*2, 1);
    for(int i = 0; i < n; ++i) {
        float x = matches[i].p.x, x_proj = matches[i].q.x;
        float y = matches[i].p.y, y_proj = matches[i].q.y;
        float data0[8] = { x, y, 1, 0, 0, 0, -x*x_proj, -y*x_proj };
        float data1[8] = { 0, 0, 0, x, y, 1, -x*y_proj, -y*y_proj };
        memcpy(*(M.data+2*i), data0, sizeof(data0));
        memcpy(*(M.data+2*i+1), data1, sizeof(data1));
        b.data[2*i][0] = x_proj;
        b.data[2*i+1][0] = y_proj;
    }
    matrix H_hat = least_squares(M, b);
    free_matrix(&M); free_matrix(&b);

    matrix empty = {0};
    if(!H_hat.data) return empty;

    matrix H = make_matrix(3, 3);

    for(int i = 0; i < M.cols; ++i) {
        H.data[i / 3][i % 3] = H_hat.data[i][0];
    }
    H.data[2][2] = 1;

    free_matrix(&H_hat);
    return H;
}


// Project an image onto a cylinder then flatten it, given focal lengths in pixels
image cylindrical_project(image m, float f)
{
    int xc = m.w / 2, yc = m.h / 2;
    int w = 2*f*atan2f(xc, f);
    image out = make_image(w, m.h, m.c);
    for(int k = 0; k < out.c; ++k) {
        #pragma omp parallel for
        for(int y = 0; y < out.h; ++y) {
            for(int x = 0; x < out.w; ++x) {
                float theta = (x - w/2) / f;
                float X = f*sinf(theta), Y = y-yc, Z = f*cosf(theta); // cylinder coordinates
                float mx = f*X/Z + xc, my = f*Y/Z + yc; // unrolled coordinates
                float val = bilinear_interpolate(m, mx, my, k);
                set_pixel(&out, x, y, k, val);
            }
        }
    }
    return out;
}

static inline matrix RANSAC(match* m, int n, float thresh, int k, int cutoff)
{
    int best = -1;
    matrix Hb = make_translation_homography(256, 0), H = make_matrix(3, 3);
    for(int i = 0; i < k; ++i) {
        shuffle_matches(m, n);
        H = compute_homography(m, 4);
        if(!H.data) continue;
        int num_inliers = model_inliers(H, m, n, thresh);
        if(num_inliers > best) {
            best = num_inliers;
            H = compute_homography(m, num_inliers);
            if(H.data) Hb = H;
            if(best > cutoff) break;
        }
    }
    printf("found %d inliers\n", best);
    free_matrix(&H);
    return Hb;
}

int model_inliers(matrix H, match* m, int n, float thresh)
{
    int count = 0;
    for(int i = 0; i < n; ++i) {
        point proj = project_point(H, m[count].p), q = m[count].q;
        float dist = point_distance(proj, q);
        if(dist < thresh) count++;
        else {
            match tmp = m[count];
            for(int j = count; j < n-1; ++j) m[j] = m[j+1];
            m[n-1] = tmp;
        }
    }
    return count;
}

// compare matches a and b
// 0 if same, 1 if a > b, -1 if a < b
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

image combine_images(image a, image b, matrix H)
{
    matrix Hinv = invert_matrix(H);

    // Project the corners of image b into image a coordinates.
    point c1 = project_point(Hinv, make_point(0, 0));
    point c2 = project_point(Hinv, make_point(b.w-1, 0));
    point c3 = project_point(Hinv, make_point(0, b.h-1));
    point c4 = project_point(Hinv, make_point(b.w-1, b.h-1));

    // Find top left and bottom right corners of image b warped into image a.
    point topleft  = { min4f(c1.x, c2.x, c3.x, c4.x), min4f(c1.y, c2.y, c3.y, c4.y) };
    point botright = { max4f(c1.x, c2.x, c3.x, c4.x), max4f(c1.y, c2.y, c3.y, c4.y) };

    // Find how big our new image should be and the offsets from image a.
    int dx = MIN(0.f, topleft.x), dy = MIN(0.f, topleft.y);
    int w = MAX(a.w, botright.x) - dx, h = MAX(a.h, botright.y) - dy;

    image out = make_image(w, h, a.c);
    // Paste image a into the new image offset by dx and dy.
    for(int k = 0; k < a.c; ++k) {
        #pragma omp parallel for
        for(int y = 0; y < a.h; ++y) {
            for(int x = 0; x < a.w; ++x) {
                set_pixel(&out, x-dx, y-dy, k, get_pixel(a, x, y, k));
            }
        }
    }
    // Paste in image b by projecting back to b and interpolate if within bounds
    #pragma omp parallel for
    for(int k = 0; k < a.c; ++k) {
        for(int y = topleft.y; y < botright.y; ++y) {
            for(int x = topleft.x; x < botright.x; ++x) {
                point p = project_point(H, make_point(x, y));
                if(p.x >= 0 && p.x < b.w && p.y >= 0 && p.y < b.h) {
                    float v = bilinear_interpolate(b, p.x, p.y, k);
                    set_pixel(&out, x-dx, y-dy, k, v);
                }
            }
        }
    }

    return out;
}

image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff, int draw_matches)
{
    int num_a=0, num_b=0, num_matches=0;
    descriptor* ad = harris_corner_detector(a, sigma, thresh, nms, &num_a);
    descriptor* bd = harris_corner_detector(b, sigma, thresh, nms, &num_b);
    match* m = match_descriptors(ad, num_a, bd, num_b, &num_matches);

    matrix H = RANSAC(m, num_matches, inlier_thresh, iters, cutoff);

    if(draw_matches) {
        draw_corners(&a, ad, num_a);
        draw_corners(&b, bd, num_b);
        image matches_image = draw_inliers(a, b, H, m, num_matches, inlier_thresh);
        save_image_png(matches_image, "matches");
    }
    free_descriptors(ad, num_a); free_descriptors(bd, num_b); free(m);

    image panorama = combine_images(a, b, H);
    return panorama;
}

// Draw the matches with inliers in green between two images.
image draw_inliers(image a, image b, matrix H, match* m, int n, float thresh)
{
    int inliers = model_inliers(H, m, n, thresh);
    image lines_image = draw_matches(a, b, m, n, inliers);
    return lines_image;
}
