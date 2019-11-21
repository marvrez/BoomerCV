#include "image.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void panorama_from_paths(char* path1, char* path2, float sigma, float thresh, float inlier_thresh,
                              int nms_window_size, int cutoff, int num_iters)
{
    image im1 = load_image_rgb(path1);
    image im2 = load_image_rgb(path2);

    double t1 = time_now();
    double t2 = time_now();
    printf("took %.3lf seconds to create panorama\n", t2-t1);

    free_image(&im1);
    free_image(&im2);
}

void run_panorama(int argc, char** argv)
{
    if(argc < 4) {
        fprintf(stderr, "usage: ./boomercv panorama path1 path2"\
                " [-sigma <sigma> -thresh <threshold> -inlier_thresh <inlier threshold> -num_iters <num iterations> -cutoff <inlier cutoff> -nms_window_size <nms window size>] \n");
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
    int nms_window_size=3, cutoff=30, num_iters=10000;
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
        }
    }

    panorama_from_paths(path1, path2, sigma, thresh, inlier_thresh, 
                        nms_window_size, cutoff, num_iters);
}
