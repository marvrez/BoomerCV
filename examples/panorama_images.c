#include "image.h"
#include "panorama.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

image panorama_from_paths(char* path1, char* path2, float sigma, float thresh, float inlier_thresh,
                              int nms_window_size, int cutoff, int num_iters, int debug, int f1, int f2)
{
    srand(time(0));
    image im1 = load_image_rgb(path1);
    image im2 = load_image_rgb(path2);

    if(f1 > 0) {
        printf("performing cylindrical projection on %s with focal length %d\n", path1, f1);
        image cyl1 = cylindrical_project(im1, f1);
        free_image(&im1);
        im1 = cyl1;
    }
    if(f2 > 0) {
        printf("performing cylindrical projection on %s with focal length %d\n", path2, f2);
        image cyl2 = cylindrical_project(im2, f2);
        free_image(&im2);
        im2 = cyl2;
    }

    image out;
    double t1 = time_now();
    out = panorama_image(im1, im2, sigma, thresh, nms_window_size,
                         inlier_thresh, num_iters, cutoff, debug);
    double t2 = time_now();
    printf("took %.3lf seconds to create panorama\n", t2-t1);

    free_image(&im1);
    free_image(&im2);

    return out;
}

void run_panorama(int argc, char** argv)
{
    if(argc < 4) {
        fprintf(stderr, "usage: ./boomercv panorama path1 path2"\
                " [-sigma <sigma> -thresh <threshold> -inlier_thresh <inlier threshold> -num_iters <num iterations> -cutoff <inlier cutoff> -nms_window_size <nms window size> -debug <1 if show debug image> -f1 <focal length image 1> -f2 <focal length image 2>] \n");
        return;
    }
    char path1[256] = {0}, path2[256] = {0};

    strcpy(path1, argv[2]);
    strcpy(path2, argv[3]);

    if(path1[0] == '\0' || path2[0] == '\0') {
        fprintf(stderr, "two image paths not provided, exiting program..\n");
        return;
    }

    float sigma=2.f, thresh=2.f, inlier_thresh=2.f;
    int nms_window_size=3, cutoff=30, num_iters=10000, debug=0, f1=0, f2=0;
    for (int i = 4; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-sigma", argv[i]) == 0) {
                sigma = atof(argv[i+1]);
            }
            else if (strcmp("-thresh", argv[i]) == 0) {
                thresh = atof(argv[i+1]);
            }
            else if (strcmp("-inlier_thresh", argv[i]) == 0) {
                inlier_thresh = atof(argv[i+1]);
            }
            else if (strcmp("-num_iters", argv[i]) == 0) {
                num_iters = atoi(argv[i+1]);
            }
            else if (strcmp("-cutoff", argv[i]) == 0) {
                cutoff = atoi(argv[i+1]);
            }
            else if (strcmp("-nms_window_size", argv[i]) == 0) {
                nms_window_size = atoi(argv[i+1]);
            }
            else if (strcmp("-debug", argv[i]) == 0) {
                debug = atoi(argv[i+1]);
            }
            else if (strcmp("-f1", argv[i]) == 0) {
                f1 = atoi(argv[i+1]);
            }
            else if (strcmp("-f2", argv[i]) == 0) {
                f2 = atoi(argv[i+1]);
            }
        }
    }

    const char* output_path = debug ? "panorama_debug" : "panorama";

    image panorama = panorama_from_paths(path1, path2, sigma, thresh, inlier_thresh,
                                         nms_window_size, cutoff, num_iters, debug, f1, f2);
    save_image_png(panorama, output_path);
    free_image(&panorama);
}
