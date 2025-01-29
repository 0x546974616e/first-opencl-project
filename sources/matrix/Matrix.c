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

#include <assert.h> // assert()
#include <ctype.h> // toupper()
#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // fprintf(), printf()
#include <stdlib.h> // malloc(), free()

#include "common/helper.h" // IN, OUT, INOUT, TR_FAILED(), RoundUp()
#include "common/OpenClBuffer.h" // OpenClBuffer{}
#include "common/OpenClContext.h" // OpenClContext{}
#include "matrix/Matrix.h" // Matrix(), Self{}

bool Matrix(Create)(
  IN OpenClContext* openCl,
  IN size_t rows,
  IN size_t columns,
  IN size_t rowPadding,
  IN size_t columnPadding,
  IN cl_mem_flags flags,
  OUT Matrix()* matrix
) {
  assert(openCl != NULL);
  assert(matrix != NULL);

  size_t size = (rows + rowPadding) * (columns + columnPadding) * sizeof(TR_MATRIX_PRECISION);
  cl_mem buffer = OpenClBuffer_Create(openCl->context, flags, size);

  if (buffer != NULL) {
    matrix->rows = rows;
    matrix->columns = columns;
    matrix->rowPadding = rowPadding;
    matrix->columnPadding = columnPadding;
    matrix->data = NULL; // Lazily allocated.
    matrix->openCl = openCl;
    matrix->buffer = buffer;
  }
  else {
    matrix->rows = 0u;
    matrix->columns = 0u;
    matrix->rowPadding = 0u;
    matrix->columnPadding = 0u;
    matrix->data = NULL;
    matrix->openCl = NULL;
    matrix->buffer = NULL;
  }

  return buffer != NULL;
}

bool Matrix(Release)(INOUT Matrix()* matrix) {
  assert(matrix != NULL);

  cl_mem buffer = matrix->buffer;
  bool success = buffer == NULL || OpenClBuffer_Release(buffer);
  if (matrix->data != NULL) {
    free(matrix->data);
  }

  matrix->rows = 0u;
  matrix->columns = 0u;
  matrix->rowPadding = 0u;
  matrix->columnPadding = 0u;
  matrix->data = NULL;
  matrix->openCl = NULL;
  matrix->buffer = NULL;

  return success;
}

bool Matrix(Display)(IN Matrix()* matrix, char const* name) {
  assert(matrix != NULL);

  static char floatingPoint[] = TR_STRINGIFY(TR_MATRIX_PRECISION);
  floatingPoint[0] = (char) toupper(floatingPoint[0]);

  size_t payload = matrix->rows * matrix->columns;
  size_t waste = ((matrix->rows + matrix->rowPadding) * (matrix->columns + matrix->columnPadding)) - payload;

  printf(
    TAB0 "Matrix%s%s:" LF

    TAB1 "Rows.(+padding).....: %zu (+%zu)" LF
    TAB1 "Columns.(+padding)..: %zu (+%zu)" LF
    TAB1 "Elements.(+padding).: %zu (+%zu)" LF
    TAB1 "Floating-Point.Type.: %s" LF
    TAB1 "Matrix.Payload......: %zu Bytes" LF
    TAB1 "Matrix.Waste........: %zu Bytes" LF
    TAB1 "Total.Size..........: %zu Bytes" LFLF

    , name != NULL ? " " : ""
    , name != NULL ? name : ""
    , matrix->rows
    , matrix->rowPadding
    , matrix->columns
    , matrix->columnPadding
    , payload
    , waste
    , floatingPoint
    , payload * sizeof(TR_MATRIX_PRECISION)
    , waste * sizeof(TR_MATRIX_PRECISION)
    , (payload + waste) * sizeof(TR_MATRIX_PRECISION)
  );

  return true;
}

bool Matrix(Random)(
  IN TR_MATRIX_PRECISION min,
  IN TR_MATRIX_PRECISION max,
  INOUT Matrix()* this
) {
  assert(this != NULL && this->data != NULL);
  (void) min; (void) max;
  (void) this;
  return false;
}

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_C
