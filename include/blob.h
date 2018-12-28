#ifndef BLOB_H
#define BLOB_H

#include "image.h"

typedef struct {
    int xmin, xmax;
    int ymin, ymax;
    int label;
} cc_label;

cc_label* cc_label_image(image m);
box* detect_blobs(image m, int* num_boxes, int(*box_filter)(int width, int height));
void draw_blob_detections(image* m, box* detections, int num_detections);

#endif
