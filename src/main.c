#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
        return 0;
    }
    return 0;
}
