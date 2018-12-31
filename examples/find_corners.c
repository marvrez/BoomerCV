#include "harris.h"
#include "image.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

image find_corners_from_path(char* path, float sigma, float thresh, int nms)
{
    image original = load_image_rgb(path);

    int num_descriptors;
    double t1 = time_now();
    descriptor* d = harris_corner_detector(original, sigma, thresh, nms, &num_descriptors);
    double t2 = time_now();
    printf("found %d corners\n", num_descriptors);
    printf("corner detection took %.3lf seconds\n", t2-t1);
    draw_corners(&original, d, num_descriptors);

    free_descriptors(d, num_descriptors);

    return original;
}

void run_corner_detection(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv corners -i <input_path> [OPTIONAL PARAMETERS: -o <output_path>]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    float sigma = 2.f, thresh = 50.f;
    int nms = 3;
    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-sigma", argv[i]) == 0) {
                sigma = atof(argv[i+1]);
            }
            else if (strcmp("-thresh", argv[i]) == 0) {
                thresh = atof(argv[i+1]);
            }
            else if (strcmp("-nms", argv[i]) == 0) {
                nms = atoi(argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image corners = find_corners_from_path(input_path, sigma, thresh, nms);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_corners");
                break;
            }
        }
    }
    save_image_png(corners, output_path);
    free_image(&corners);
}
