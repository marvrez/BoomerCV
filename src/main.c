#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void run_resize(int argc, char** argv);
extern void run_grayscale(int argc, char** argv);
extern void run_binarize(int argc, char** argv);
extern void run_filter(int argc, char** argv);
extern void run_find_lines(int argc, char** argv);

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
    else {
        fprintf(stderr, "%s is not a valid option\n", argv[1]);
    }

    return 0;
}
