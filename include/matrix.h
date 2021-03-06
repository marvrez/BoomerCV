#ifndef MATRIX_H
#define MATRIX_H

typedef struct matrix {
    int rows, cols;
    float** data;
} matrix;

matrix make_matrix(int rows, int cols);
matrix make_identity(int n);
matrix make_translation_homography(float dx, float dy);
matrix copy_matrix(matrix m);
void free_matrix(matrix* m);

matrix transpose_matrix(matrix m);
matrix multiply_matrix(matrix a, matrix b);
matrix invert_matrix(matrix m);

matrix least_squares(matrix M, matrix b);

#endif
