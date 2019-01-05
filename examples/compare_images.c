#include "image.h"
#include "phash.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

void compare_images_from_path(char* path1, char* path2)
{
    image im1 = load_image_rgb(path1);
    image im2 = load_image_rgb(path2);

    double t1 = time_now();
    uint64_t ph1 = phash(im1);
    uint64_t ph2 = phash(im2);
    double t2 = time_now();
    printf("took %.3lf seconds to create hashes\n", t2-t1);
    printf("%s --> 0x%" PRIx64 "\n", path1, ph1);
    printf("%s --> 0x%" PRIx64 "\n", path2, ph2);

    int dist = phash_compare(ph1, ph2);
    printf("bit difference: %d bits\n", dist);

    free_image(&im1);
    free_image(&im2);
}

void run_phash(int argc,  char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: ./boomercv phash <path1> <path2>\n");
        return;
    }
    char path1[256] = {0}, path2[256] = {0};

    strcpy(path1, argv[2]);
    strcpy(path2, argv[3]);

    if(path1[0] == '\0' || path2[0] == '\0') {
        fprintf(stderr, "two image paths not provided, exiting program..\n");
        return;
    }
    compare_images_from_path(path1, path2);
}
