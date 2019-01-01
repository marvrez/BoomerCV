#include "image.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void run_webcam(int argc, char** argv)
{
    #ifndef OPENCV
        fprintf(stderr, "opencv must be enabled to use the webcam viewer\n");
        return;
    #else
    if(argc < 1) {
        fprintf(stderr, "usage: ./boomercv webcam [OPTIONAL PARAMETERS: -id <device_id> -w <viewer_width> -h <viewer_height>]\n");
        return;
    }
    int device_id = 0;
    int w = 0, h = 0;
    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-id", argv[i]) == 0) {
                device_id = atoi(argv[i+1]);
            }
            else if (strcmp("-w", argv[i]) == 0) {
                w = atoi(argv[i+1]);
            }
            else if (strcmp("-h", argv[i]) == 0) {
                h = atoi(argv[i+1]);
            }
        }
    }

    void* cap = open_video_stream(0, device_id, w, h, 0);
    const char* windowname = "webcam memes";

    printf("ctrl-c or esc to exit the viewer\n");
    setbuf(stdout, NULL);
    double fps = 0.f, start_time = 0.f;
    while(1) {
        image in = get_image_from_stream(cap);
        fps = 1.f / (time_now() - start_time);
        start_time = time_now();
        printf("fps: %.1f\r", fps);
        int key = show_image(in, windowname, 5);
        free_image(&in);

        if(key != -1 && key % 256 == 27) break;
    }
    #endif
}
