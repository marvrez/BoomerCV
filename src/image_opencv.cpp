#ifdef OPENCV

#include "image.h"

#include <stdlib.h>

#include "opencv2/opencv.hpp"

using namespace cv;

extern "C" {

static inline Mat image_to_mat(image m)
{
    image copy = copy_image(m);
    clamp_image(&copy);
    if(m.c == 3) rgb_to_bgr(&copy);
    unsigned char* data = (unsigned char*)malloc(m.w*m.h*m.c);
    #pragma omp parallel for
    for(int y = 0; y < m.h; ++y) {
        for(int x = 0; x < m.w; ++x) {
            for(int c = 0; c < m.c; ++c) {
                float val = copy.data[c*m.h*m.w + y*m.w + x];
                data[y*m.w*m.c + x*m.c + c] = (unsigned char)(val*255);
            }
        }
    }
    Mat out(m.h, m.w, CV_MAKETYPE(CV_8U, m.c), data);
    free_image(&copy);
    free(data);
    return out;
}

static inline image mat_to_image(Mat m)
{
    int w = m.cols, h = m.rows, c = m.channels();
    image out = make_image(w, h, c);
    unsigned char* data = (unsigned char*)m.data;
    int step = m.step;

    const float normalizing_factor =  1 / 255.f;
    #pragma omp parallel for
    for(int i = 0; i < h; ++i) {
        for(int k = 0; k < c; ++k) {
            for(int j = 0; j < w; ++j) {
                out.data[k*w*h + i*w + j] = data[i*step + j*c + k]*normalizing_factor;
            }
        }
    }
    bgr_to_rgb(&out);
    return out;
}

image get_image_from_stream(void* cap)
{
    Mat m;
    *((VideoCapture*)cap) >> m;
    if(m.empty()) return make_empty_image(0, 0, 0);
    return mat_to_image(m);
}

void* open_default_cam()
{
    return open_video_stream(0, 0, 0, 0, 0);
}

void* open_video_stream(const char* filename, int device_id, int w, int h, int fps)
{
    VideoCapture* cap;
    if(filename) cap = new VideoCapture(filename);
    else cap = new VideoCapture(device_id);
    if(!cap->isOpened()) return NULL;
    if(w) cap->set(CAP_PROP_FRAME_WIDTH, w);
    if(h) cap->set(CAP_PROP_FRAME_HEIGHT, h);
    if(fps) cap->set(CAP_PROP_FPS, fps);
    return (void*)cap;
}

int show_image(image m, const char* windowname, int ms)
{
    Mat out = image_to_mat(m);
    imshow(windowname, out);
    int c = waitKey(ms);
    if (c != -1) c = c % 256;
    return c;
}

}
#endif
