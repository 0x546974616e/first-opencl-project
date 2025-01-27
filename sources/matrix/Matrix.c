#ifndef TR_MATRIX_C
#ifndef TR_MATRIX_PRECISION
#  define TR_MATRIX_PRECISION float
#  include "matrix/Matrix.c"
#  undef TR_MATRIX_PRECISION
#    define TR_MATRIX_PRECISION double
#    include "matrix/Matrix.c"
#    undef TR_MATRIX_PRECISION
#      define TR_MATRIX_C
#else // TR_MATRIX_PRECISION

#include <CL/opencl.h> // Khronos API

#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t

#include "common/helper.h" // IN, OUT, INOUT
#include "matrix/Matrix.h" // Matrix(), Self{}

bool Matrix(NewWithHostMemory)(IN size_t rows, IN size_t columns, OUT Matrix()* matrix) {
  (void) matrix;
  (void) rows; (void) columns;
  return false;
}

bool Matrix(NewWithDeviceMemory)(IN size_t rows, IN size_t columns, OUT Matrix()* matrix) {
  (void) matrix;
  (void) rows; (void) columns;
  return false;
}

bool Matrix(Release)(INOUT Matrix()* this) {
  (void) this;
  return false;
}

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_C
