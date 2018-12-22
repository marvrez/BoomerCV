#include "filter.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    SMOOTH,
    SHARP,
    ERODE,
    DILATE,
    SOBEL,
    SOBEL_COLOR,
    EQUALIZE_HISTOGRAM,
    SKELETONIZE,
} filter_type;

filter_type get_filter_type(char* type_str)
{
    if(strcmp(type_str, "smooth") == 0) return SMOOTH;
    if(strcmp(type_str, "equalize") == 0) return EQUALIZE_HISTOGRAM;
    if(strcmp(type_str, "skeletonize") == 0) return SKELETONIZE;
    if(strcmp(type_str, "sharp") == 0) return SHARP;
    if(strcmp(type_str, "sobel") == 0) return SOBEL;
    if(strcmp(type_str, "sobel_color") == 0) return SOBEL_COLOR;
    if(strcmp(type_str, "dilate") == 0) return DILATE;
    if(strcmp(type_str, "erode") == 0) return ERODE;
    return SMOOTH;
}

image filter_image_from_path(char* path, filter_type type)
{
    image original = load_image_rgb(path);
    image out = make_empty_image(original.w, original.h, original.c), *s, binarized, gray;

    double t1 = time_now();
    switch(type) {
        case SMOOTH:
            out = smoothen_image(original, 7);
            break;
        case SHARP:
            out = sharpen_image(original);
            break;
        case ERODE:
            out = erode_image(original, 10);
            break;
        case DILATE:
            out = dilate_image(original, 10);
            break;
        case SOBEL:
            s = sobel_image(original);
            out = s[0];
            free_image(&s[1]);
            free(s);
            break;
        case SOBEL_COLOR:
            out = colorize_sobel(original);
            break;
        case EQUALIZE_HISTOGRAM:
            out = equalize_histogram(original);
            break;
        case SKELETONIZE:
            gray = rgb_to_grayscale(original);
            binarized = threshold_image(gray, 0.5f);
            out = skeletonize_image(binarized);
            free_image(&gray);
            free_image(&binarized);
            break;
        default:
            out = smoothen_image(original, 10);
            break;
    }
    double t2 = time_now();
    printf("filtering took %.3lf seconds\n", t2-t1);

    free_image(&original);
    return out;
}

void run_filter(int argc,  char** argv)
{
    if(argc < 6) {
        fprintf(stderr, "usage: ./boomercv filter -i <input_path> -t [smooth, skeletonize, sharp, sobel, sobel_color, dilate, erode, equalize] [OPTIONAL PARAMETERS: -o <output_path>]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    filter_type type = SMOOTH;

    for(int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-t", argv[i]) == 0) {
                type = get_filter_type(argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image filtered_image = filter_image_from_path(input_path, type);
    if(output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_filtered");
                break;
            }
        }
    }
    save_image_png(filtered_image, output_path);
    free_image(&filtered_image);
}
