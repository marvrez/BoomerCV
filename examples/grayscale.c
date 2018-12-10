#include "image.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

image grayscale_image_from_path(char* path)
{
    image original = load_image_rgb(path);
    image gray = make_empty_image(original.w, original.h, original.c);

    double t1 = time_now();
    gray = rgb_to_grayscale(original);
    double t2 = time_now();
    printf("grayscaling took %.3lf seconds\n", t2-t1);

    free_image(&original);
    return gray;
}

void run_grayscale(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv grayscale -i <input_path> [OPTIONAL PARAMETERS: -o <output_path>]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image gray = grayscale_image_from_path(input_path);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_grayscaled");
                break;
            }
        }
    }
    save_image_png(gray, output_path);
    free_image(&gray);
}
