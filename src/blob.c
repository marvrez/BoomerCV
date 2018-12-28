#include "blob.h"

#include "draw.h"
#include "stretchy_buffer.h"

#include <stdlib.h>

cc_label* cc_label_image(image m)
{
	int w = m.w, h = m.h, n = m.w*m.h;
    cc_label* out_labels = NULL;

    int* queue = (int*)malloc(n*sizeof(int));
    int* labels = (int*)malloc(n*sizeof(int));
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) labels[i] = -1;

    int count = 0;
    for(int i = 0; i < n; ++i) {
        if(m.data[i] <= 0.f) continue;
        if(labels[i] == -1) {
            queue[0] = i;
            int queue_count = 1;
            cc_label label = { w, 0, h, 0, -1 };
            while(queue_count > 0) {
                // extract from queue and grow component
                int p = queue[--queue_count];
                if(labels[p] != -1) continue;
                labels[p] = label.label = count;

                int x = p % w, y = p / w;
                label.xmin = (x < label.xmin) ? x : label.xmin;
                label.ymin = (y < label.ymin) ? y : label.ymin;
                label.xmax = (x > label.xmax) ? x : label.xmax;
                label.ymax = (y > label.ymax) ? y : label.ymax;

                int neighbors[] = { p+w, p-w, p+1, p-1, p+1+w, p-1+w, p+1-w, p-1-w };
                for (int j = 0; j < 8; ++j) {
                    int q = neighbors[j];
                    if (m.data[q] > 0.f && labels[q] == -1) {
                        queue[queue_count++] = q;
                    }
                }
            }
            count++;
            sb_push(out_labels, label);
        }
    }
    free(queue);
    free(labels);
	return out_labels;
}

box* detect_blobs(image m, int* num_boxes, int(*box_filter)(int width, int height))
{
	if (!m.data || m.c != 1) {
		if (num_boxes) *num_boxes = 0;
		return 0;
	}
    cc_label* labels = cc_label_image(m);
    int num_labels = (int)sb_count(labels);

    box detection;
    box* out_boxes = NULL;
    #pragma omp parallel for schedule(dynamic) private(detection)
    for(int i = 0; i < num_labels; ++i) {
        cc_label label = labels[i];
        if(label.label == -1) continue;
        int allow = 1;
        int h = label.ymax - label.ymin, w = label.xmax - label.xmin;
        if(box_filter) {
            allow = box_filter(w, h);
        }
        if(allow) {
            detection.score = 0;
            detection.name = "blob";
            detection.x = label.xmin, detection.y = label.ymin;
            detection.w = w, detection.h = h;
            #pragma omp critical
            sb_push(out_boxes, detection);
        }
    }
    sb_free(labels);

    *num_boxes = (int)sb_count(out_boxes);
	return out_boxes;
}

void draw_blob_detections(image* m, box* detections, int num_detections)
{
    #pragma omp parallel for
    for (int i = 0; i < num_detections; ++i) {
        draw_bbox_width(m, detections[i], 8, 0, 191, 255);
    }
}
