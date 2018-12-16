#include "filter.h"

#include <string.h>

image dilate_image(image m, int times)
{
    if (m.c != 1) return make_empty_image(0,0,0);

    image out = make_image(m.w, m.h, m.c);
    image tmp = copy_image(m);
    while(times--) {
        for (int y = 0; y < m.h; ++y) {
            for (int x = 0; x < m.w; ++x) {
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
        for (int y = 0; y < m.h; ++y) {
            for (int x = 0; x < m.w; ++x) {
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
