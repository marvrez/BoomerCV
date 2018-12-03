#include "utils.h"

#include <time.h>
#include <sys/time.h>

double time_now()
{
    struct timeval time;
    if (gettimeofday(&time, NULL)) return 0;
    return (double)time.tv_sec + (double)time.tv_usec * 1e-6;
}

int clamp(int val, int min, int max)
{
    int clamped_val = val;
    if (val < min) clamped_val = min;
    if (val > max) clamped_val = max;
    return clamped_val;
}
