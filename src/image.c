#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

image make_image(int w, int h, int c)
{
    image m = make_empty_image(w, h, c);
    m.data = calloc(w*h*c, sizeof(float));
    return m;
}

image make_empty_image(int w, int h, int c)
{
    image m;
    m.data = NULL;
    m.h = h;
    m.w = w;
    m.c = c;
    return m;
}

image copy_image(image m) 
{
    image copy = m;
	copy.data = calloc(m.w*m.h*m.c, sizeof(float));
	if (copy.data && m.data) {
		memcpy(copy.data, m.data, m.w*m.h*m.c * sizeof(float));
	}
    return copy;
}

void free_image(image* m)
{
    if (m->data) {
        free(m->data);
    }
}

image get_channel(image m, int c)
{
    image out = make_image(m.w, m.h, 1);
    if (out.data && c >= 0 && c < m.c) {
        for (int i = 0; i < m.h*m.w; ++i) {
            out.data[i] = m.data[i + c*m.w*m.h];
        }
    }
    return out;
}

unsigned char* get_image_data_hwc(image m)
{
    unsigned char* data = 0;
    if (m.data) {
        data = calloc(m.w*m.h*m.c, sizeof(unsigned char));
        if (data) {
            for (int k = 0; k < m.c; ++k) {
                for (int i = 0; i < m.w*m.h; ++i) {
                    data[i*m.c + k] = (unsigned char)(255 * m.data[i + k*m.w*m.h]);
                }
            }
        }
    }
    return data;
}
