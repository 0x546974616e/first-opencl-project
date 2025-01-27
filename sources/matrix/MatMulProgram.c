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

static bool RUNMATMULPROGRAM(TR_MATRIX_PRECISION)(IN MatMulContext* this) {
  assert(matrixMatMulStart <= matrixMatMulEnd);
  assert(this != NULL);

  // TODO: Check multiplication overflow?
  // TOOD: What about endianness?

  (void) matrixMatMulStart;
  (void) matrixMatMulEnd;

  cl_int error;
  cl_context context = this->openCl.context;
  cl_device_id device = this->openCl.device;
  cl_command_queue queue = this->openCl.queue;
  cl_program program = NULL;

  Matrix() Aa; (void) Aa;

  size_t ASize = (this->M + this->paddingM) * (this->N + this->paddingN);
  size_t BSize = (this->N + this->paddingN) * (this->P + this->paddingP);
  size_t CSize = (this->M + this->paddingM) * (this->P + this->paddingP);

  assert(ASize % this->blockSize == 0u);
  assert(BSize % this->blockSize == 0u);
  assert(CSize % this->blockSize == 0u);

  // TODO: If CL_MEM_USE_HOST_PTR, alignment & exclusivity requirements
  // [...] make sure host_ptr is aligned and a multiple of a certain size (for Intel
  // devices, aligned on a 4 KB boundary and a multiple of 64 bytes).
  size_t ABytes = sizeof(TR_MATRIX_PRECISION) * ASize;
  size_t BBytes = sizeof(TR_MATRIX_PRECISION) * BSize;
  size_t CBytes = sizeof(TR_MATRIX_PRECISION) * CSize;

  TR_MATMUL_LOG(this, 2, "A (" TR_STRINGIFY(TR_MATRIX_PRECISION) ") = %zu bytes", ABytes);
  TR_MATMUL_LOG(this, 2, "B (" TR_STRINGIFY(TR_MATRIX_PRECISION) ") = %zu bytes", BBytes);
  TR_MATMUL_LOG(this, 2, "C (" TR_STRINGIFY(TR_MATRIX_PRECISION) ") = %zu bytes", CBytes);
  TR_MATMUL_LOG(this, 2,
    "Total waste (" TR_STRINGIFY(TR_MATRIX_PRECISION) ") = %zu bytes"
    , MatMulContext_ComputeWaste(this)
  );

  TR_MATRIX_PRECISION* A = malloc(ABytes); if (NULL == A) { goto outA; }
  TR_MATRIX_PRECISION* B = malloc(BBytes); if (NULL == B) { goto outB; }
  TR_MATRIX_PRECISION* C = malloc(CBytes); if (NULL == C) { goto outC; }

  TR_MATMUL_LOG(this, 1, "Create OpenCL Program.");
  size_t sourceLength = (size_t) (matrixMatMulEnd - matrixMatMulStart); // TODO: Overflow.
  program = clCreateProgramWithSource(context, 1, &matrixMatMulStart, &sourceLength, &error);
  if (error != CL_SUCCESS || program == NULL) {
    TR_FAILED("clCreateProgramWithSource()", error);
    goto outProgram;
  }

  #define TR_OPTIONS_SIZE 64
  TR_MATMUL_LOG(this, 1, "Generate Build Options.");
  char buildOptions[TR_OPTIONS_SIZE + 1] = { 0x0 };
  int written = snprintf(buildOptions, TR_OPTIONS_SIZE, "-DMATMUL_BLOCKSIZE=%zu", this->blockSize);
  buildOptions[TR_OPTIONS_SIZE] = 0x0; // To be sure to avoid overflow.
  if (written >= TR_OPTIONS_SIZE) {
    TR_ERROR("The build options buffer is too small, abort.");
    goto outBuild;
  }

  TR_MATMUL_LOG(this, 1, "Build OpenCL Program.");
  error = clBuildProgram(program, 0, NULL, buildOptions, NULL, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clBuildProgram()", error);
    OpenClContext_DisplayBuildError(program, &this->openCl);
    goto outBuild;
  }

  (void) device; (void) queue;

  // https://stackoverflow.com/questions/57854782/how-opencl-memory-transfer-functions-work

  // // https://stackoverflow.com/questions/26517114/how-to-compile-opencl-project-with-kernels
  // // Create the memory buffers
  // cl::Buffer bufferA=cl::Buffer(context, CL_MEM_READ_ONLY, N_ELEMENTS * sizeof(int));
  // cl::Buffer bufferB=cl::Buffer(context, CL_MEM_READ_ONLY, N_ELEMENTS * sizeof(int));
  // cl::Buffer bufferC=cl::Buffer(context, CL_MEM_WRITE_ONLY, N_ELEMENTS * sizeof(int));
  // // Copy the input data to the input buffers using the command queue.
  // queue.enqueueWriteBuffer( bufferA, CL_FALSE, 0, N_ELEMENTS * sizeof(int), A.get() );
  // queue.enqueueWriteBuffer( bufferB, CL_FALSE, 0, N_ELEMENTS * sizeof(int), B.get() );
  // // ...
  // // Copy the output data back to the host
  // queue.enqueueReadBuffer( bufferC, CL_TRUE, 0, N_ELEMENTS * sizeof(int), C.get() );


  /*
  If you already have the data and want to load the data into an OpenCL buffer
  object, then use CL_MEM_USE_HOST_PTR with a buffer allocated at a 4096 byte
  boundary (aligned to a page and cache line boundary) and a total size that is
  a multiple of 64 bytes (cache line size).
  */

  // cl_mem
  // clGetMemObjectInfo()
  // cl_mem = clCreateBuffer()
            // flags |= CL_MEM_READ_ONLY;
            // flags |= CL_MEM_READ_WRITE;
            // flags |= CL_MEM_USE_HOST_PTR;

  // clEnqueueReadBuffer
  // clEnqueueWriteBuffer
  // clEnqueueFillBuffer
  // clEnqueueMapBuffer, clEnqueueUnmapMemObject
  // clEnqueueNDRangeKernel
  // clEnqueueWaitForEvents

  // cl_context context = this->oclContext->context;
  // cl::Context context(deviceType);
  // cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);
  // cl::Program program(context, std::string(matrixMatMulSource), false); // true = build

  // clRetainProgram(program)
  // clReleaseProgram(program)
  // clRetainKernel(kernel)
  // clReleaseKernel(kernel)
  // clRetainEvent(event)
  // clReleaseEvent(event)

  // row-major order

outBuild:
outProgram:
  if (program != NULL) {
    TR_MATMUL_LOG(this, 2, "Release OpenCL Program.");
    if (CL_SUCCESS != (error = clReleaseProgram(program))) {
      TR_FAILED("clReleaseProgram()", error);
    }
  }

outC: if (C != NULL) { free(C); }
outB: if (B != NULL) { free(B); }
outA: if (A != NULL) { free(A); }

  return false;
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
