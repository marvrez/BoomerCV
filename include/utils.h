#ifndef UTILS_H
#define UTILS_H

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

double time_now();
int clamp(int val, int min, int max);
float max3f(float a, float b, float c);
float min3f(float a, float b, float c);

#endif
