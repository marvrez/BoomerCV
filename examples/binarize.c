#include "image.h"

#include <stdio.h>
#include <string.h>

typedef enum {
    OTSU,
    BINARY,
} binarize_type;

binarize_type get_binarize_type(char* type_str)
{
    if(strcmp(type_str, "otsu") == 0) return OTSU;
    if(strcmp(type_str, "binary") == 0) return BINARY;
    return BINARY;
}

image binarize_from_path(char* path, binarize_type type)
{
    image original = load_image_rgb(path);
    image gray = rgb_to_grayscale(original);
    free_image(&original);

    image binarized = make_empty_image(gray.w, gray.h, gray.c);
    double t1 = time_now();
    switch(type) {
        case OTSU:
            binarized = otsu_binarize_image(gray);
            break;
        case BINARY:
        default:
            binarized = binarize_image(gray, 0);
            break;
    }
    double t2 = time_now();
    printf("binarizing took %.3lf seconds\n", t2-t1);

    free_image(&gray);
    return binarized;
}

void run_binarize(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv binarize -i <input_path> -o [OPTIONAL PARAMETERS: <output_path> -t <binarize_type>]\n");
        fprintf(stderr, "<binarize_type> is one of [otsu, binarize]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    binarize_type type = BINARY;

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-t", argv[i]) == 0) {
                type = get_binarize_type(argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image binarized = binarize_from_path(input_path, type);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_binarized");
                break;
            }
        }
    }
    save_image_png(binarized, output_path);
    free_image(&binarized);
}
