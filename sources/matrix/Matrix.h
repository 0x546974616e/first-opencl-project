#ifndef TR_MATRIX_H
#ifndef TR_MATRIX_PRECISION
#  define TR_MATRIX_PRECISION float
#  include "matrix/Matrix.h"
#  undef TR_MATRIX_PRECISION
#    define TR_MATRIX_PRECISION double
#    include "matrix/Matrix.h"
#    undef TR_MATRIX_PRECISION
#      define TR_MATRIX_H
#else // TR_MATRIX_PRECISION

#include <CL/opencl.h> // Khronos API

#include <stdbool.h> // bool
#include <stddef.h> // size_t

#include "common/helper.h" // IN, OUT, INOUT, TR_CONCAT2(), TR_JOIN2()
#include "common/OpenClContext.h" // OpenClContext{}

#undef Matrix
#undef TR_float
#undef TR_double
#  define TR_float 1
#  define TR_double 2
#  if TR_float == TR_CONCAT2(TR_, TR_MATRIX_PRECISION)
#    define Matrix(suffix) TR_JOIN2(_, MatrixFloat, suffix)
#  elif TR_double == TR_CONCAT2(TR_, TR_MATRIX_PRECISION)
#    define Matrix(suffix) TR_JOIN2(_, MatrixDouble, suffix)
#  else // TR_float || TR_double
#    error TR_MATRIX_PRECISION := float | double
#  endif // TR_float || TR_double
#undef TR_float
#undef TR_double

typedef struct Matrix() {
  size_t rows, rowPadding;
  size_t columns, columnPadding;
  TR_MATRIX_PRECISION* data; // Lazily allocated
  OpenClContext* openCl;
  cl_mem buffer;
} Matrix();

///
/// Create a new matrix (row-major order).
///
/// The size (in bytes) of the matrix is given by:
/// - `(rows + rowPadding) * (columns + columnPadding) * sizeof(float | double)`
///
/// Flags may be one of the following values:
/// - `CL_MEM_READ_WRITE`
/// - `CL_MEM_WRITE_ONLY`
/// - `CL_MEM_READ_ONLY`
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `context` is not NULL.
/// @pre `matrix` is not NULL.
/// @pre The matrix dimensions are non-zero.
///
bool Matrix(Create)(
  IN OpenClContext* context,
  IN size_t rows,
  IN size_t columns,
  IN size_t rowPadding,
  IN size_t columnPadding,
  IN cl_mem_flags flags,
  OUT Matrix()* matrix
);

///
/// Release the matrix.
///
/// It is safe to give a matrix after an unsuccessful call to Matrix(Create)().
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `matrix` is not NULL.
///
bool Matrix(Release)(INOUT Matrix()* matrix);

///
/// Display informations about the given matrix.
///
/// @pre `matrix` is not NULL.
/// @pre `name` may be NULL.
/// @post Display on stdout.
///
bool Matrix(Display)(IN Matrix()* matrix, char const* name);

// Matrix(Write)
// Matrix(Read)

///
///
///
bool Matrix(Random)(
  IN TR_MATRIX_PRECISION min,
  IN TR_MATRIX_PRECISION max,
  INOUT Matrix()* matrix
);

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_H
