#include "draw.h"

#include <math.h>
#include <stdlib.h>

void draw_box(image* m, int x1, int y1, int x2, int y2, float r, float g, float b)
{
    r /= 255.f, g /= 255.f, b /= 255.f;
    if(x1 < 0) { x1 = 0; } if(x1 >= m->w) { x1 = m->w - 1; }
    if(x2 < 0) { x2 = 0; } if(x2 >= m->w) { x2 = m->w - 1; }

    if(y1 < 0) { y1 = 0; } if(y1 >= m->h) { y1 = m->h - 1; }
    if(y2 < 0) { y2 = 0; } if(y2 >= m->h) { y2 = m->h - 1; }

    for (int i = x1; i <= x2; ++i) {
        m->data[i + y1*m->w + 0*m->w*m->h] = r;
        m->data[i + y2*m->w + 0*m->w*m->h] = r;

        m->data[i + y1*m->w + 1*m->w*m->h] = g;
        m->data[i + y2*m->w + 1*m->w*m->h] = g;

        m->data[i + y1*m->w + 2*m->w*m->h] = b;
        m->data[i + y2*m->w + 2*m->w*m->h] = b;
    }
    for (int i = y1; i <= y2; ++i) {
        m->data[x1 + i*m->w + 0*m->w*m->h] = r;
        m->data[x2 + i*m->w + 0*m->w*m->h] = r;

        m->data[x1 + i*m->w + 1*m->w*m->h] = g;
        m->data[x2 + i*m->w + 1*m->w*m->h] = g;

        m->data[x1 + i*m->w + 2*m->w*m->h] = b;
        m->data[x2 + i*m->w + 2*m->w*m->h] = b;
    }
}

void draw_circle(image* m, int x0, int y0, int radius, float r, float g, float b)
{
    #define plot(x, y) set_pixel(m,x,y,0,r);set_pixel(m,x,y,1,g);set_pixel(m,x,y,2,b);
    r /= 255., g /= 255.f, b /= 255.;

    int f = 1 - radius;
    int ddF_x = 0, ddF_y = -2 * radius;
    int x = 0, y = radius;

    plot(x0, y0 + radius); plot(x0, y0 - radius);
    plot(x0 + radius, y0); plot(x0 - radius, y0);

    while (x < y) {
        if (f >= 0) {
            --y;
            ddF_y += 2;
            f += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f += ddF_x + 1;
        plot(x0 + x, y0 + y); plot(x0 - x, y0 + y);
        plot(x0 + x, y0 - y); plot(x0 - x, y0 - y);
        plot(x0 + y, y0 + x); plot(x0 - y, y0 + x);
        plot(x0 + y, y0 - x); plot(x0 - y, y0 - x);
    }
    #undef plot
}

void draw_line(image* m, int x1, int y1, int x2, int y2, float r, float g, float b)
{
    r /= 255.f, g /= 255.f, b /= 255.f;

    if(x1 < 0) { x1 = 0; } if(x1 >= m->w) { x1 = m->w - 1; }
    if(x2 < 0) { x2 = 0; } if(x2 >= m->w) { x2 = m->w - 1; }

    if(y1 < 0) { y1 = 0; } if(y1 >= m->h) { y1 = m->h - 1; }
    if(y2 < 0) { y2 = 0; } if(y2 >= m->h) { y2 = m->h - 1; }

	int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int error = (dx >= dy ? dx : -dy) / 2;

	for (;;) {
        m->data[x1 + y1*m->w + 0*m->w*m->h] = r;
        m->data[x1 + y1*m->w + 1*m->w*m->h] = g;
        m->data[x1 + y1*m->w + 2*m->w*m->h] = b;

		if (x1 >= x2 && y1 >= y2) break;
		int current_error = error;
		if (current_error > -dx) { error -= dy; x1 += sx; }
		if (current_error <  dy) { error += dx; y1 += sy; }
	}
}

void draw_grid(image* m, float x_min, float y_min, float x_max, float y_max, int steps, float r, float g, float b)
{
    for (int i = 0; i <= steps; ++i) {
        draw_line(m, x_min, y_min + (y_max-y_min)*i/steps,
                     x_max, y_min + (y_max-y_min)*i/steps,
                     r, g, b);

        draw_line(m, x_min + (x_max-x_min)*i/steps, y_min,
                     x_min + (x_max-x_min)*i/steps, y_max,
                     r, g, b);
    }
}

void draw_bbox(image* m, box bbox, float r, float g, float b)
{
    draw_box(m, bbox.x, bbox.y, bbox.x + bbox.w, bbox.y + bbox.h, r, g, b);
}

void draw_bbox_width(image* m, box bbox, int width, float r, float g, float b)
{
    for (int i = 0; i < width; ++i) {
        draw_box(m, bbox.x + i, bbox.y + i, (bbox.x + bbox.w) - i, (bbox.y + bbox.h) - i, r, g, b);
    }
}

void draw_grid_width(image* m, float x_min, float y_min, float x_max, float y_max, int steps, int width, float r, float g, float b)
{
    for(int w = 0; w <= width; ++w) {
        for (int i = 0; i <= steps; ++i) {
            int sign = i != steps ? 1 : -1;
            draw_line(m, x_min, y_min + sign*w + (y_max-y_min)*i/steps,
                         x_max, y_min + sign*w + (y_max-y_min)*i/steps,
                         r, g, b);

            draw_line(m, x_min + w + (x_max-x_min)*i/steps, y_min,
                         x_min + w + (x_max-x_min)*i/steps, y_max,
                         r, g, b);
        }
    }
}

void draw_circle_thickness(image* m, int x0, int y0, int radius, int width, float r, float g, float b)
{

    for (int i = 0; i < width; ++i) {
        draw_circle(m, x0, y0, radius - i, r, g, b);
    }
}
