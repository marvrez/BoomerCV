#include "image.h"
#include "flow.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void optical_flow_webcam(int smooth, int stride, int div)
{
#ifdef OPENCV
    void* cap = open_default_cam();

    image prev = get_image_from_stream(cap);
    image prev_resized = nn_resize(prev, prev.w/div, prev.h/div);
    image cur = get_image_from_stream(cap);
    image resized = nn_resize(cur, cur.w/div, cur.h/div);

    double fps = 0.f, start_time = 0.f;
    while(cur.data) {
        image cur_copy = copy_image(cur);
        image v = optical_flow_images(resized, prev_resized, smooth, stride);
        fps = 1.f / (time_now() - start_time);
        printf("fps: %.1f\r", fps);
        draw_flow(&cur_copy, v, smooth*div*2);
        int key = show_image(cur_copy, "optical flow memes", 5);
        start_time = time_now();

        free_image(&v);
        free_image(&cur_copy);
        free_image(&prev);
        free_image(&prev_resized);

        prev = cur, prev_resized = resized;
        if(key != -1 && key % 256 == 27) break;
        cur = get_image_from_stream(cap);
        resized = nn_resize(cur, cur.w/div, cur.h/div);
    }
#else
    fprintf(stderr, "must compile with opencv\n");
#endif
}

void run_flow(int argc, char** argv)
{
    #ifndef OPENCV
        fprintf(stderr, "opencv must be enabled to use the optical flow viewer\n");
        return;
    #else
    if(argc < 1) {
        fprintf(stderr, "usage: ./boomercv flow [OPTIONAL PARAMETERS: -smooth <smoothness> -stride <stride> -div <div>]\n");
        return;
    }
    int smooth = 15, stride = 4, div = 8;
    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-smooth", argv[i]) == 0) {
                smooth = atoi(argv[i+1]);
            }
            else if (strcmp("-stride", argv[i]) == 0) {
                stride = atoi(argv[i+1]);
            }
            else if (strcmp("-div", argv[i]) == 0) {
                div = atoi(argv[i+1]);
            }
        }
    }
    printf("ctrl-c to exit the viewer\n");
    setbuf(stdout, NULL);
    optical_flow_webcam(smooth, stride, div);
    #endif
}
