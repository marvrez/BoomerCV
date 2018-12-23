#include "hough.h"
#include "draw.h"

#include "stretchy_buffer.h"

#include <stdlib.h>
#include <math.h>

#define DEG2RAD 0.017453293f

accumulator hough_transform(image m)
{
    accumulator a;
    //Create the accumulator
    float hough_h = sqrtf(2.f)*(m.h > m.w ? m.h : m.w) / 2.f;
    a.h = hough_h*2, a.w = 180;

    a.histogram = (unsigned int*)calloc(a.w*a.h, sizeof(unsigned int));

    float center_x = m.w/2.f, center_y = m.h/2.f;
    for(int y = 0; y < m.h; ++y) {
        for(int x = 0; x < m.w; ++x) {
            if(m.data[y*m.w + x] == 1.f) {
                for(int t = 0; t < 180; ++t) {
                    float r = ((x - center_x)*cosf(t*DEG2RAD)) + ((y - center_y)*sinf(t*DEG2RAD));
                    ++a.histogram[(int)((round(r + hough_h)*180.f)) + t];
                }
            }
        }
    }
    return a;
}

static inline int find_local_maximum(accumulator a, int r, int t)
{
    int max = (int)a.histogram[r*a.w + t];
    for (int ly = -4; ly <= 4; ++ly) {
        for (int lx = -4; lx <= 4; ++lx) {
            if ((ly + r >= 0 && ly + r < a.h) && (lx + t >= 0 && lx + t < a.w)) {
                if ((int)a.histogram[(r + ly)*a.w + (t + lx)] > max) {
                    max = (int)a.histogram[(r + ly)*a.w + (t + lx)];
                    ly = lx = 5;
                }
            }
        }
    }
    return max;
}

line* hough_line_detect(image m, int threshold, int* num_lines)
{
    // Require a binary image output from canny
    if (!m.data || m.c != 1) {
        *num_lines = 0;
        return 0;
    }
    accumulator a = hough_transform(m);
    line* lines = 0, l;

    if(threshold < 1) threshold = m.w > m.h ? m.w / 3 : m.h / 3;
    for (int r = 0; r < a.h; ++r) {
        for (int t = 0; t < a.w; ++t) {
            if((int)a.histogram[r*a.w + t] < threshold) continue;
            int max = find_local_maximum(a, r, t);
            if(max > (int)a.histogram[r*a.w + t]) continue;

            int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
            if (t >= 45 && t <= 135) {
                // y = (r - x cos(t)) / sin(t)
                x1 = 0;
                y1 = ((float)(r - (a.h/2.f)) - ((x1 - (m.w/2.f))*cosf(t*DEG2RAD))) / sinf(t*DEG2RAD) + (m.h/2.f);
                x2 = m.w - 0;
                y2 = ((float)(r - (a.h/2.f)) - ((x2 - (m.w/2.f))*cosf(t*DEG2RAD))) / sinf(t*DEG2RAD) + (m.h/2.f);
            }
            else {
                // x = (r - y sin(t)) / cos(t)
                y1 = 0;
                x1 = ((float)(r - (a.h/2.f)) - ((y1 - (m.h/2.f))*sinf(t*DEG2RAD)))/cosf(t*DEG2RAD) + (m.w/2.f);
                y2 = m.h - 0;
                x2 = ((float)(r - (a.h/2.f)) - ((y2 - (m.h/2.f))*sinf(t*DEG2RAD)))/cosf(t*DEG2RAD) + (m.w/2.f);
            }
            l.start.x = x1, l.start.y = y1;
            l.end.x   = x2, l.end.y   = y2;
            sb_push(lines, l);
        }
    }
    free(a.histogram);
    *num_lines = (int)sb_count(lines);
    return lines;
}

void draw_hough_lines(image* m, line* lines, int num_lines, float r, float g, float b)
{
    for(int i = 0; i < num_lines; ++i) {
        line l = lines[i];
        draw_line(m, l.start.x, l.start.y, l.end.x, l.end.y, r, g, b);
    }
}
