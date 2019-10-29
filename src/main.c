#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void run_resize(int argc, char** argv);
extern void run_grayscale(int argc, char** argv);
extern void run_binarize(int argc, char** argv);
extern void run_filter(int argc, char** argv);
extern void run_find_lines(int argc, char** argv);
extern void run_blob_detect(int argc, char** argv);
extern void run_corner_detection(int argc, char** argv);
extern void run_webcam(int argc, char** argv);
extern void run_flow(int argc, char** argv);
extern void run_phash(int argc, char** argv);
extern void run_rotate(int argc,  char** argv);

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "resize") == 0) {
        run_resize(argc, argv);
    }
    else if (strcmp(argv[1], "grayscale") == 0) {
        run_grayscale(argc, argv);
    }
    else if (strcmp(argv[1], "binarize") == 0) {
        run_binarize(argc, argv);
    }
    else if (strcmp(argv[1], "filter") == 0) {
        run_filter(argc, argv);
    }
    else if (strcmp(argv[1], "lines") == 0) {
        run_find_lines(argc, argv);
    }
    else if (strcmp(argv[1], "blobs") == 0) {
        run_blob_detect(argc, argv);
    }
    else if (strcmp(argv[1], "corners") == 0) {
        run_corner_detection(argc, argv);
    }
    else if (strcmp(argv[1], "webcam") == 0) {
        run_webcam(argc, argv);
    }
    else if (strcmp(argv[1], "flow") == 0) {
        run_flow(argc, argv);
    }
    else if (strcmp(argv[1], "phash") == 0) {
        run_phash(argc, argv);
    }
    else if (strcmp(argv[1], "rotate") == 0) {
        run_rotate(argc, argv);
    }
    else {
        fprintf(stderr, "%s is not a valid option\n", argv[1]);
    }

    return 0;
}
