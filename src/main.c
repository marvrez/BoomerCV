#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void run_resize(int argc, char** argv);

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "resize") == 0) {
        run_resize(argc, argv);
    }
    else {
        fprintf(stderr, "%s is not a valid option\n", argv[1]);
    }

    return 0;
}
