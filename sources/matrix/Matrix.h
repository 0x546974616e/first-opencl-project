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

  /// Host pointer or mapped device memory.
  TR_MATRIX_PRECISION* pointer;
  cl_mem memory;
} Matrix();

///
///
///
bool Matrix(NewWithHostMemory)(
  IN size_t rows,    IN size_t rowPadding,
  IN size_t columns, IN size_t columnPadding,
  IN OpenClContext* context,
  OUT Matrix()* matrix
);

///
///
///
bool Matrix(NewWithDeviceMemory)(
  IN size_t rows,    IN size_t rowPadding,
  IN size_t columns, IN size_t columnPadding,
  IN OpenClContext* context,
  OUT Matrix()* matrix
);

///
///
///
bool Matrix(Release)(INOUT Matrix()* matrix);

///
///
///
bool Matrix(RandomInitialization)(
  IN TR_MATRIX_PRECISION min,
  IN TR_MATRIX_PRECISION max,
  INOUT Matrix()* matrix
);

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_H
