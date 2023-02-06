/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#define min(x, y) ((x) < (y)? (x):(y))

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans32(int M, int N, int A[N][M], int B[M][N]);
void trans_std(int M, int N, int A[N][M], int B[M][N]);
void trans64(int M, int N, int A[N][M], int B[M][N]);
static void print_matrix(int M, int N, int C[M][N]);
void imporved_trans_64(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32) {
        trans32(M, N, A, B);
    } else if (M == 64){
        imporved_trans_64(M, N, A, B);
    } else {
        trans_std(M, N, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

char trans32_desc[] = "A good solution for 32x32 matrix";
void trans32(int M, int N, int A[N][M], int B[M][N]) {
    int in, jn, row_index, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, temprow_index;
    for (in = 0; in < N; in += 8) {
        for (jn = 0; jn < M; jn += 8) {
            for (row_index = 0; row_index < 8; row_index++) {
                temprow_index = in + row_index;
                tmp0 = A[temprow_index][jn + 0];
                tmp1 = A[temprow_index][jn + 1];
                tmp2 = A[temprow_index][jn + 2];
                tmp3 = A[temprow_index][jn + 3];
                tmp4 = A[temprow_index][jn + 4];
                tmp5 = A[temprow_index][jn + 5];
                tmp6 = A[temprow_index][jn + 6];
                tmp7 = A[temprow_index][jn + 7];
                B[jn + 0][temprow_index] = tmp0;
                B[jn + 1][temprow_index] = tmp1;
                B[jn + 2][temprow_index] = tmp2;
                B[jn + 3][temprow_index] = tmp3;
                B[jn + 4][temprow_index] = tmp4;
                B[jn + 5][temprow_index] = tmp5;
                B[jn + 6][temprow_index] = tmp6;
                B[jn + 7][temprow_index] = tmp7;
            }
        }
    }
}

char trans_std_desc[] = "Standard matrix for testing memory structure";
void trans_std(int M, int N, int A[N][M], int B[M][N]) {
    int in, jn, row_index, col_index, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (in = 0; in < N; in += 8) {
        for (jn = 0; jn < M; jn += 8) {
            if (min(M - jn, 8) == 8 && min(N - in, 8) == 8) {
                for (row_index = 0; row_index < 8; row_index++) {
                    col_index = in + row_index;
                    tmp0 = A[col_index][jn + 0];
                    tmp1 = A[col_index][jn + 1];
                    tmp2 = A[col_index][jn + 2];
                    tmp3 = A[col_index][jn + 3];
                    tmp4 = A[col_index][jn + 4];
                    tmp5 = A[col_index][jn + 5];
                    tmp6 = A[col_index][jn + 6];
                    tmp7 = A[col_index][jn + 7];
                    B[jn + 0][col_index] = tmp0;
                    B[jn + 1][col_index] = tmp1;
                    B[jn + 2][col_index] = tmp2;
                    B[jn + 3][col_index] = tmp3;
                    B[jn + 4][col_index] = tmp4;
                    B[jn + 5][col_index] = tmp5;
                    B[jn + 6][col_index] = tmp6;
                    B[jn + 7][col_index] = tmp7;
                }
            } else {
                for (row_index = 0; row_index < min(N - in, 8); row_index++) {
                    for (col_index = 0; col_index < min(M - jn, 8); col_index++) {
                        B[jn + col_index][in + row_index] = A[in + row_index][jn + col_index];
                    }
                }
            }
        }
    }
}
char trans64_desc[] = "4x4 submatrix blocking";
void trans64(int M, int N, int A[N][M], int B[M][N]) {
    int in, jn, row_index, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (in = 0; in < N; in += 8) {
        for (jn = 0; jn < M; jn += 8) {
            for (row_index = 0; row_index < 4; row_index++) {
                tmp0 = A[row_index + in][jn];
                tmp1 = A[row_index + in][jn + 1];
                tmp2 = A[row_index + in][jn + 2];
                tmp3 = A[row_index + in][jn + 3];
                tmp4 = A[row_index + in][jn + 4];
                tmp5 = A[row_index + in][jn + 5];
                tmp6 = A[row_index + in][jn + 6];
                tmp7 = A[row_index + in][jn + 7];
                B[jn + 0][row_index + in] = tmp0;
                B[jn + 1][row_index + in] = tmp1;
                B[jn + 2][row_index + in] = tmp2;
                B[jn + 3][row_index + in] = tmp3;
                B[jn + 0][row_index + in + 4] = tmp4;
                B[jn + 1][row_index + in + 4] = tmp5;
                B[jn + 2][row_index + in + 4] = tmp6;
                B[jn + 3][row_index + in + 4] = tmp7;
            }
            for (row_index = 4; row_index < 8; row_index++) {
                tmp0 = A[row_index + in][jn];
                tmp1 = A[row_index + in][jn + 1];
                tmp2 = A[row_index + in][jn + 2];
                tmp3 = A[row_index + in][jn + 3];
                tmp4 = A[row_index + in][jn + 4];
                tmp5 = A[row_index + in][jn + 5];
                tmp6 = A[row_index + in][jn + 6];
                tmp7 = A[row_index + in][jn + 7];
                B[jn + 4][row_index + in - 4] = tmp0;
                B[jn + 5][row_index + in - 4] = tmp1;
                B[jn + 6][row_index + in - 4] = tmp2;
                B[jn + 7][row_index + in - 4] = tmp3;
                B[jn + 4][row_index + in] = tmp4;
                B[jn + 5][row_index + in] = tmp5;
                B[jn + 6][row_index + in] = tmp6;
                B[jn + 7][row_index + in] = tmp7;
            }
            for (row_index = 7; row_index > 3; row_index--) {
                tmp0 = B[jn + row_index][in];
                tmp1 = B[jn + row_index][in + 1];
                tmp2 = B[jn + row_index][in + 2];
                tmp3 = B[jn + row_index][in + 3];
                tmp4 = B[jn + row_index - 4][in + 4];
                tmp5 = B[jn + row_index - 4][in + 5];
                tmp6 = B[jn + row_index - 4][in + 6];
                tmp7 = B[jn + row_index - 4][in + 7];
                B[jn + row_index - 4][in + 4] = tmp0;
                B[jn + row_index - 4][in + 5] = tmp1;
                B[jn + row_index - 4][in + 6] = tmp2;
                B[jn + row_index - 4][in + 7] = tmp3;
                B[jn + row_index][in] = tmp4;
                B[jn + row_index][in + 1] = tmp5;
                B[jn + row_index][in + 2] = tmp6;
                B[jn + row_index][in + 3] = tmp7;
            }
        }
    }
}

void imporved_trans_64(int M, int N, int A[N][M], int B[M][N]) {
    int in, jn, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int row_index;
    for (in = 0; in < N; in += 8) {
        for (jn = 0; jn < M; jn += 8) {
            for (row_index = 0; row_index < 4; row_index++) {
                tmp0 = A[row_index + in][jn];
                tmp1 = A[row_index + in][jn + 1];
                tmp2 = A[row_index + in][jn + 2];
                tmp3 = A[row_index + in][jn + 3];
                tmp4 = A[row_index + in][jn + 4];
                tmp5 = A[row_index + in][jn + 5];
                tmp6 = A[row_index + in][jn + 6];
                tmp7 = A[row_index + in][jn + 7];
                B[jn + 0][row_index + in] = tmp0;
                B[jn + 1][row_index + in] = tmp1;
                B[jn + 2][row_index + in] = tmp2;
                B[jn + 3][row_index + in] = tmp3;
                B[jn + 0][row_index + in + 4] = tmp4;
                B[jn + 1][row_index + in + 4] = tmp5;
                B[jn + 2][row_index + in + 4] = tmp6;
                B[jn + 3][row_index + in + 4] = tmp7;
            }
            for (row_index = 0; row_index < 4; row_index ++) {
                tmp0 = A[4 + in][jn + row_index];
                tmp1 = A[5 + in][jn + row_index];
                tmp2 = A[6 + in][jn + row_index];
                tmp3 = A[7 + in][jn + row_index];

                tmp4 = B[jn + row_index][4 + in];
                tmp5 = B[jn + row_index][5 + in];
                tmp6 = B[jn + row_index][6 + in];
                tmp7 = B[jn + row_index][7 + in];

                B[jn + row_index][4 + in] = tmp0;
                B[jn + row_index][5 + in] = tmp1;   
                B[jn + row_index][6 + in] = tmp2;
                B[jn + row_index][7 + in] = tmp3;

                B[jn + 4 + row_index][in] = tmp4;
                B[jn + 4 + row_index][in + 1] = tmp5;
                B[jn + 4 + row_index][in + 2] = tmp6;
                B[jn + 4 + row_index][in + 3] = tmp7;
            }
            for (row_index = 4; row_index < 8; ++row_index) {
                tmp0 = A[in + row_index][jn + 4];
                tmp1 = A[in + row_index][jn + 5];
                tmp2 = A[in + row_index][jn + 6];
                tmp3 = A[in + row_index][jn + 7];
                B[jn + 4][in + row_index] = tmp0;
                B[jn + 5][in + row_index] = tmp1;
                B[jn + 6][in + row_index] = tmp2;
                B[jn + 7][in + row_index] = tmp3;
            }
        }
    }
}
/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    
    int C[8][8], D[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            C[i][j] = i * 8 + j;
        }
    }
    imporved_trans_64(8, 8, C, D);
    print_matrix(8, 8, D);
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_std, trans_std_desc); 

}

static void print_matrix(int M, int N, int C[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

