#include "image.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

image rotate_image_from_path(char* path, int direction)
{
    image original = load_image_rgb(path);
    image out = rotate_image_left_or_right(original, direction);
    free_image(&original);
    return out;
}

void run_rotate(int argc,  char** argv)
{
    if(argc < 5) {
        fprintf(stderr, "usage: ./boomercv rotate -i <input_path> [-r to rotate right] [-l to rotate left] [OPTIONAL PARAMETERS: -o <output_path>]\n");
        return;
    }

    char input_path[256] = {0}, output_path[512] = {0};
    int direction = 0;
    for(int i = 1; i < argc; ++i) {
        if (strcmp("-i", argv[i]) == 0) {
            strcpy(input_path, argv[i+1]);
        }
        else if (strcmp("-o", argv[i]) == 0) {
            strcpy(output_path, argv[i+1]);
        }
        else if (strcmp("-l", argv[i]) == 0) {
            direction = 0;
        }
        else if (strcmp("-r", argv[i]) == 0) {
            direction = 1;
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }

    image rotated_image = rotate_image_from_path(input_path, direction);
    if(output_path[0] == '\0') {
        strcat(output_path, input_path);
        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_rotated");
                break;
            }
        }
    }
    save_image_png(rotated_image, output_path);
    free_image(&rotated_image);
}
