/*
 * IMPORTANT NOTE:
 *
 * This file leverages recursive `#include` to define the MatMul program for
 * single- and double-precision floating-point format. It is divided into four
 * sections with different behaviors: 1) "MatMul-Start" section which is called
 * once at the beginning of the recursive includes; 2) "MatMul-Includes" section
 * which actually includes the file recursively, two times for the "MatMul-Body"
 * section with `float` and `double` floating-point types, and a third times for
 * the "MatMul-End" section; 3) "MatMul-Body" section with `TR_MATRIX_PRECISION`
 * defined once as `float` and a second time as `double`; 4) and "MatMul-End"
 * section which called at the end of the recursive procedure. You can check
 * symbols with `nm build/matrix/MatMulProgram.o`.
 */

#ifndef TR_MATRIX_MATMULPROGRAM_C
#ifndef TR_MATRIX_PRECISION

// ╔╦╗┌─┐┌┬┐╔╦╗┬ ┬┬    ╔═╗┌┬┐┌─┐┬─┐┌┬┐
// ║║║├─┤ │ ║║║│ ││  ──╚═╗ │ ├─┤├┬┘ │
// ╩ ╩┴ ┴ ┴ ╩ ╩└─┘┴─┘  ╚═╝ ┴ ┴ ┴┴└─ ┴

#include <CL/opencl.h> // Khronos API

#include <assert.h> // assert()
#include <stdbool.h> // bool, true, false
#include <stdio.h> // printf()

#include "common/helper.h" // IN, TR_CONCAT, TR_PRINT(), TR_FAILED()
#include "common/OpenCl.h" // TR_OPENCL_IMPORT()
#include "common/OpenClProgram.h" // Self()
#include "matrix/MatMulContext.h" // Self{}
#include "matrix/MatMulProgram.h" // Self{}

#define MATMULBLOCKSIZE 16u // TOOD: 16 so far.
#define RUNMATMULPROGRAM(TYPE) TR_JOIN2(_, RunMatMulProgram, TYPE)

// Define matrixMatMulStart and matrixMatMulEnd.
TR_OPENCL_IMPORT(matrix, MatMul)

static bool RUNMATMULPROGRAM(float)(IN MatMulContext* context);
static bool RUNMATMULPROGRAM(double)(IN MatMulContext* context);

// ╔╦╗┌─┐┌┬┐╔╦╗┬ ┬┬    ╦┌┐┌┌─┐┬  ┬ ┬┌┬┐┌─┐┌─┐
// ║║║├─┤ │ ║║║│ ││  ──║││││  │  │ │ ││├┤ └─┐
// ╩ ╩┴ ┴ ┴ ╩ ╩└─┘┴─┘  ╩┘└┘└─┘┴─┘└─┘╶┴┘└─┘└─┘

#define TR_MATRIX_PRECISION float
#include "matrix/MatMulProgram.c"
#undef TR_MATRIX_PRECISION
#  define TR_MATRIX_PRECISION double
#  include "matrix/MatMulProgram.c"
#  undef TR_MATRIX_PRECISION
#    define TR_MATRIX_MATMULPROGRAM_C
#    include "matrix/MatMulProgram.c"
#else // TR_MATRIX_PRECISION

// ╔╦╗┌─┐┌┬┐╔╦╗┬ ┬┬    ╔╗ ┌─┐┌┬┐┬ ┬
// ║║║├─┤ │ ║║║│ ││  ──╠╩╗│ │ ││└┬┘
// ╩ ╩┴ ┴ ┴ ╩ ╩└─┘┴─┘  ╚═╝└─┘╶┴┘ ┴

#include "matrix/Matrix.h" // Matrix(), Self{}

// TODO: Check multiplication overflow?
// TOOD: What about endianness?

static bool RUNMATMULPROGRAM(TR_MATRIX_PRECISION)(IN MatMulContext* this) {
  assert(matrixMatMulStart <= matrixMatMulEnd);
  assert(this != NULL);

  bool success = true;
  cl_program program = NULL;
  Matrix() A, B, C;

  #define TR_OPTIONS_SIZE 64
  TR_MATMUL_LOG(this, 1, "Generate Build Options.");
  char buildOptions[TR_OPTIONS_SIZE + 1] = { 0x0 };
  int written = snprintf(buildOptions, TR_OPTIONS_SIZE, "-DMATMUL_BLOCKSIZE=%zu", this->blockSize);
  buildOptions[TR_OPTIONS_SIZE] = 0x0; // To be sure to avoid overflow.
  if (written >= TR_OPTIONS_SIZE) {
    TR_ERROR("The build options buffer is too small, abort.");
    success = false;
    goto outOptions;
  }

  TR_MATMUL_LOG(this, 1, "Load OpenCL Program.");
  size_t sourceLength = (size_t) (matrixMatMulEnd - matrixMatMulStart); // TODO: Overflow.
  program = OpenClProgram_Build(&this->openCl, buildOptions, matrixMatMulStart, sourceLength);
  if (program == NULL) {
    success = false;
    goto outProgram;
  }

  TR_MATMUL_LOG(this, 1, "Create Matrixes.");
  success = success && Matrix(Create)(&this->openCl, this->M, this->N, this->paddingM, this->paddingN, CL_MEM_READ_ONLY,  &A);
  success = success && Matrix(Create)(&this->openCl, this->N, this->P, this->paddingN, this->paddingP, CL_MEM_READ_ONLY,  &B);
  success = success && Matrix(Create)(&this->openCl, this->M, this->P, this->paddingM, this->paddingP, CL_MEM_WRITE_ONLY, &C);

  if (!success) {
    goto outMatrixes;
  }

  if (this->verbose >= 2) {
    printf(LF);
    Matrix(Display)(&A, "A");
    Matrix(Display)(&B, "B");
    Matrix(Display)(&C, "C");
  }

  // clRetainKernel(kernel)
  // clReleaseKernel(kernel)
  // clRetainEvent(event)
  // clReleaseEvent(event)

outMatrixes:
  TR_MATMUL_LOG(this, 1, "Release Matrixes.");
  Matrix(Release)(&A);
  Matrix(Release)(&B);
  Matrix(Release)(&C);

  TR_MATMUL_LOG(this, 1, "Release OpenCL Program.");
  OpenClProgram_Release(program);

outProgram:
outOptions:
  return success;
}

// ╔╦╗┌─┐┌┬┐╔╦╗┬ ┬┬    ╔═╗┌┐┌┌┬┐
// ║║║├─┤ │ ║║║│ ││  ──║╣ │││ ││
// ╩ ╩┴ ┴ ┴ ╩ ╩└─┘┴─┘  ╚═╝┘└┘╶┴┘

#endif // TR_MATRIX_PRECISION
#else // TR_MATRIX_MATMULPROGRAM_C

bool MatMulProgram_Run(IN MatMulContext* context) {
  assert(context != NULL);
  return context->openCl.fp64Extension
    ? RUNMATMULPROGRAM(double)(context)
    : RUNMATMULPROGRAM(float)(context);
}

#endif // TR_MATRIX_MATMULPROGRAM_C
