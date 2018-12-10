#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "image.h"

image threshold_image(image m, float thresh);
image otsu_binarize_image(image m);
image binarize_image(image m, int reverse);

#endif
