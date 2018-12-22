#include "image.h"
#include "hough.h"
#include "filter.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

image find_lines_from_path(char* path, int threshold)
{
    image original = load_image_rgb(path);
    line* lines;
    int num_lines;

    double t1 = time_now();
    image* s = sobel_image(original); // TODO: swap with canny edge detection when implemented
    lines = hough_line_detect(s[0], threshold, &num_lines);
    draw_hough_lines(&original, lines, num_lines, 1.f, 1.f, 0.2f);
    printf("%d\n", num_lines);
    double t2 = time_now();
    printf("line detection took %.3lf seconds\n", t2-t1);

    free_image(&s[0]); free_image(&s[1]);
    free(s);
    return original;
}

void run_find_lines(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv lines -i <input_path> [OPTIONAL PARAMETERS: -o <output_path>, -t <threshold>]\n");
        return;
    }
    char input_path[256] = {0}, output_path[512] = {0};
    int threshold = 0;

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (strcmp("-i", argv[i]) == 0) {
                strcpy(input_path, argv[i+1]);
            }
            else if (strcmp("-o", argv[i]) == 0) {
                strcpy(output_path, argv[i+1]);
            }
            else if (strcmp("-t", argv[i]) == 0) {
                threshold = atoi(argv[i+1]);
            }
        }
    }
    if(input_path[0] == '\0') {
        fprintf(stderr, "image path not provided, exiting program..\n");
        return;
    }
    image hough_img = find_lines_from_path(input_path, threshold);
    if (output_path[0] == '\0') {
        strcat(output_path, input_path);

        int n = strlen(input_path);
        for(int i = n-1; i >= 0; --i) {
            if(output_path[i] == '.') {
                strcpy(output_path + i, "_lines");
                break;
            }
        }
    }
    save_image_png(hough_img, output_path);
    free_image(&hough_img);
}
