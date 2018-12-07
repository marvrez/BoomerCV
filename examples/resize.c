#include "image.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    BILINEAR,
    NEAREST_NEIGHBOUR,
} resize_type;

resize_type get_resize_type(char* type_str)
{
    if(strcmp(type_str, "bilinear") == 0) return BILINEAR;
    if(strcmp(type_str, "nearest") == 0) return NEAREST_NEIGHBOUR;
    return BILINEAR;
}

image (*get_resize_function(resize_type type))(image, int, int)
{
    switch(type) {
        case BILINEAR:
            return bilinear_resize;
        case NEAREST_NEIGHBOUR:
            return nn_resize;
    }
    return bilinear_resize;
}

image resize_image_from_path(char* path, int output_w, int output_h, resize_type type)
{
    image original = load_image_rgb(path);
    image resized = make_empty_image(output_w, output_h, 3);

    image (*resize_func)(image, int, int) = get_resize_function(type);

    double t1 = time_now();
    resized = resize_func(original, output_w, output_h);
    double t2 = time_now();
    printf("resizing took %.3lf seconds\n", t2-t1);

    free_image(&original);
    return resized;
}

void run_resize(int argc,  char** argv)
{
    if(argc < 6) {
        fprintf(stderr, "usage: ./boomercv resize -i <input_path> -h <height> -w <width> -o [OPTIONAL]<output_path> -t [OPTIONAL]<resize_type>\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    int width = -1, height = -1;
    resize_type type = BILINEAR;

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-w", argv[i]) == 0) {
                width = atoi(argv[i+1]);
            }
            else if (strcmp("-h", argv[i]) == 0) {
                height = atoi(argv[i+1]);
            }
            else if (strcmp("-t", argv[i]) == 0) {
                type = get_resize_type(argv[i+1]);
            }
        }
    }
    image resized = resize_image_from_path(input_path, width, height, type);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_resized");
                break;
            }
        }
    }
    save_image_png(resized, output_path);
    free_image(&resized);
}
