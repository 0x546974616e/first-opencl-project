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

#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t

#include "common/helper.h" // IN, OUT, INOUT, TR_CONCAT()

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

  // size_t X, paddingX;
  // size_t Y, paddingY;

  TR_MATRIX_PRECISION* pointer; // host pointer or mapped
  cl_mem memory;
} Matrix();

///
///
///
bool Matrix(NewWithHostMemory)(
  IN size_t rows,
  IN size_t columns,
  OUT Matrix()* matrix
);

///
///
///
bool Matrix(NewWithDeviceMemory)(IN size_t rows, IN size_t columns, OUT Matrix()* matrix);

///
///
///
bool Matrix(Release)(INOUT Matrix()* matrix);

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_H
