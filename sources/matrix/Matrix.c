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
#include <stdlib.h> // malloc(), free(), rand()

#include "common/helper.h" // IN, OUT, INOUT, TR_FAILED(), RoundUp()
#include "common/OpenClBuffer.h" // OpenClBuffer{}
#include "common/OpenClContext.h" // OpenClContext{}
#include "matrix/Matrix.h" // Matrix(), Self{}

///
/// Returns tha matrix size (with padding) in bytes.
///
/// @pre `this` is not NULL.
///
static size_t Matrix(TotalSize)(IN Matrix()* this) {
  size_t size = (this->rows + this->rowPadding) * (this->columns + this->columnPadding);
  return size * sizeof(TR_MATRIX_PRECISION);
}

///
/// Allocates matrix data (with padding) if not already done.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `this` is not NULL.
///
static bool Matrix(AllocateData)(INOUT Matrix()* this) {
  if (this->data == NULL) {
    this->data = malloc(Matrix(TotalSize)(this));
  }

  return this->data != NULL;
}

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

bool Matrix(Display)(IN Matrix()* matrix, IN char const* name) {
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

bool Matrix(DisplaySample)(IN Matrix()* matrix, IN size_t rows, IN size_t columns) {
  assert(matrix != NULL);

  rows = MIN(rows, matrix->rows);
  columns = MIN(columns, matrix->columns);
  TR_MATRIX_PRECISION* data = (TR_MATRIX_PRECISION*) matrix->data;
  size_t width = matrix->columns + matrix->columnPadding;

  if (data != NULL) {
    for (size_t i = 0u; i < rows; ++i) {
      printf("[%zu]", i);
      for (size_t j = 0u; j < columns; ++j) {
        // printf(" % 10.6f", data[i * width + j]);
        printf(" %f", data[i * width + j]);
      }
      printf(" ..." LF);
    }
    printf("[...]" LFLF);
  }

  return true;
}

bool Matrix(Random)(
  INOUT Matrix()* matrix,
  IN TR_MATRIX_PRECISION minimum,
  IN TR_MATRIX_PRECISION maximun
) {
  assert(matrix != NULL);
  bool success = Matrix(AllocateData)(matrix);

  if (success) {
    TR_MATRIX_PRECISION* data = (TR_MATRIX_PRECISION*) matrix->data;
    size_t width = matrix->columns + matrix->columnPadding;
    for (size_t i = 0u; i < matrix->rows; ++i) {
      for (size_t j = 0u; j < matrix->columns; ++j) {
        TR_MATRIX_PRECISION value = (TR_MATRIX_PRECISION) rand() / (TR_MATRIX_PRECISION) RAND_MAX;
        value = value * (maximun - minimum) + minimum;
        data[i * width + j] = value;
      }
    }
  }

  return success;
}

bool Matrix(Write)(IN Matrix()* matrix) {
  assert(matrix != NULL && matrix->data != NULL);
  return OpenClBuffer_Write(
    matrix->openCl->queue,
    matrix->buffer,
    matrix->data,
    Matrix(TotalSize)(matrix)
  );
}

bool Matrix(Read)(INOUT Matrix()* matrix) {
  assert(matrix != NULL);
  bool success = Matrix(AllocateData)(matrix);
  return success && OpenClBuffer_Read(
    matrix->openCl->queue,
    matrix->buffer,
    matrix->data,
    Matrix(TotalSize)(matrix)
  );
}

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_C
