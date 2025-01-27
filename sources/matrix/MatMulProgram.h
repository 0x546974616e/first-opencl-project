#ifndef TR_MATRIX_MATMULPROGRAM_H
#define TR_MATRIX_MATMULPROGRAM_H

#include <stdbool.h> // bool, true, false

#include "common/helper.h" // IN
#include "matrix/MatMulContext.h" // Self{}

///
/// Runs the matrix multiplication with OpenCL.
///
bool MatMulProgram_Run(IN MatMulContext* context);

#endif // TR_MATRIX_MATMULPROGRAM_H
