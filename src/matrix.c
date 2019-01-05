#include "matrix.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

matrix make_matrix(int rows, int cols)
{
    matrix m;
    m.rows = rows, m.cols = cols;
    m.data = calloc(m.rows, sizeof(float*));
    for(int i = 0; i < m.rows; ++i) {
        m.data[i] = calloc(m.cols, sizeof(float));
    }
    return m;
}

matrix make_identity(int n)
{
    matrix m = make_matrix(n, n);
    m.rows = n, m.cols = n;
    m.data = calloc(n, sizeof(float*));
    for(int i = 0; i < n; ++i) {
        m.data[i] = calloc(n, sizeof(float));
        m.data[i][i] = 1.f;
    }
    return m;
}

matrix copy_matrix(matrix m)
{
    matrix c;
    c.rows = m.rows, c.cols = m.cols;
    c.data = calloc(c.rows, sizeof(float*));
    for(int i = 0; i < m.rows; ++i) {
        c.data[i] = calloc(c.cols, sizeof(float));
        for(int j = 0; j < m.cols; ++j) {
            c.data[i][j] = m.data[i][j];
        }
    }
    return c;
}

void free_matrix(matrix* m)
{
    if (m->data) {
        for(int i = 0; i < m->rows; ++i) {
            free(m->data[i]);
        }
        free(m->data);
    }
}

matrix transpose_matrix(matrix m)
{
    matrix t;
    t.rows = m.cols, t.cols = m.rows;
    t.data = calloc(t.rows, sizeof(float*));
    for(int i = 0; i < t.rows; ++i) {
        t.data[i] = calloc(t.cols, sizeof(float));
        for(int j = 0; j < t.cols; ++j) {
            t.data[i][j] = m.data[j][i];
        }
    }
    return t;
}

matrix multiply_matrix(matrix a, matrix b)
{
    assert(a.cols == b.rows);
    matrix result = make_matrix(a.rows, b.cols);
    for(int i = 0; i < result.rows; ++i) {
        for(int j = 0; j < result.cols; ++j) {
            for(int k = 0; k < a.cols; ++k) {
                result.data[i][j] += a.data[i][k]*b.data[k][j];
            }
        }
    }
    return result;
}

static inline matrix augment_matrix(matrix m)
{
    matrix augmented = make_matrix(m.rows, m.cols*2);
    for(int i = 0; i < m.rows; ++i) {
        for(int j = 0; j < m.cols; ++j) {
            augmented.data[i][j] = m.data[i][j];
        }
    }
    for(int j = 0; j < m.rows; ++j) {
        augmented.data[j][j+m.cols] = 1.f;
    }
    return augmented;
}

matrix invert_matrix(matrix m)
{
    matrix none = {0};
    if(m.rows != m.cols) {
        fprintf(stderr, "matrix must be square\n");
        return none;
    }
    matrix c = augment_matrix(m);

    for(int k = 0; k < c.rows; ++k) {
        float p = 0.f;
        int index = -1;
        for(int i = k; i < c.rows; ++i) {
            float val = fabs(c.data[i][k]);
            if(val > p) {
                p = val;
                index = i;
            }
        }
        if(index == -1) {
            fprintf(stderr, "matrix is not invertible\n");
            free_matrix(&c);
            return none;
        }

        float* tmp = c.data[index];
        c.data[index] = c.data[k];
        c.data[k] = tmp;

        float val = c.data[k][k];
        c.data[k][k] = 1.f;
        for(int j = k+1; j < c.cols; ++j) {
            c.data[k][j] /= val;
        }
        for(int i = k+1; i < c.rows; ++i) {
            float s = -c.data[i][k];
            c.data[i][k] = 0.f;
            for(int j = k+1; j < c.cols; ++j) {
                c.data[i][j] +=  s*c.data[k][j];
            }
        }
    }
    for(int k = c.rows-1; k > 0; --k) {
        for(int i = 0; i < k; ++i){
            float s = -c.data[i][k];
            c.data[i][k] = 0.f;
            for(int j = k+1; j < c.cols; ++j) {
                c.data[i][j] += s*c.data[k][j];
            }
        }
    }

    matrix inv = make_matrix(m.rows, m.cols);
    for(int i = 0; i < m.rows; ++i) {
        for(int j = 0; j < m.cols; ++j) {
            inv.data[i][j] = c.data[i][j+m.cols];
        }
    }
    free_matrix(&c);
    return inv;
}
