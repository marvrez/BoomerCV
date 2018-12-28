#include "blob.h"
#include "filter.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int box_filter(int width, int height)
{
    return !(width < 40 || height < 40 || (width >= 400 && height >= 400));
}

image detect_blobs_from_path(char* path, int num_dilations)
{
    image original = load_image_rgb(path);
    image gray = rgb_to_grayscale(original);
    int num_boxes;

    double t1 = time_now();
    image binary = binarize_image(gray, 0);
    image dilated = dilate_image(binary, num_dilations);
    box* detections = detect_blobs(dilated, &num_boxes, &box_filter);
    double t2 = time_now();
    printf("blob detection took %.3lf seconds\n", t2-t1);
    printf("found %d blobs\n", num_boxes);
    
    draw_blob_detections(&original, detections, num_boxes);

    if(detections) free((int*)detections - 2);
    free_image(&gray);
    free_image(&binary);
    free_image(&dilated);

    return original;
}

void run_blob_detect(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv blobs -i <input_path> [OPTIONAL PARAMETERS: -o <output_path> -n <number_of_dilations>]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    int num_dilations = 12;

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-n", argv[i]) == 0) {
                num_dilations = atoi(argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image detections = detect_blobs_from_path(input_path, num_dilations);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_blobs");
                break;
            }
        }
    }
    save_image_png(detections, output_path);
    free_image(&detections);
}
