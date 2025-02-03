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

  TR_MATMUL_LOG(this, 1, "Create Matrixes A, B and C.");
  success = success && Matrix(Create)(&this->openCl, this->M, this->N, this->paddingM, this->paddingN, CL_MEM_READ_ONLY , &A);
  success = success && Matrix(Create)(&this->openCl, this->N, this->P, this->paddingN, this->paddingP, CL_MEM_READ_ONLY , &B);
  success = success && Matrix(Create)(&this->openCl, this->M, this->P, this->paddingM, this->paddingP, CL_MEM_WRITE_ONLY, &C);

  TR_MATMUL_LOG(this, 1, "Initialize Matrixes A and B.");
  success = success && Matrix(Random)(&A, 0, 1);
  success = success && Matrix(Random)(&B, 0, 1);

  TR_MATMUL_LOG(this, 1, "Write Matrixes A and B to OpenCL Device.");
  success = success && Matrix(Write)(&A);
  success = success && Matrix(Write)(&B);

  if (!success) {
    TR_ERROR("One of the matrix procedures failed.");
    goto outMatrixes;
  }

  if (this->verbose >= 2) {
    printf(LF);
    Matrix(Display)(&A, "A"); if (this->verbose >= 3) { Matrix(DisplaySample)(&A, 5, 8); }
    Matrix(Display)(&B, "B"); if (this->verbose >= 3) { Matrix(DisplaySample)(&B, 5, 8); }
    Matrix(Display)(&C, "C");
  }

  // https://github.com/KhronosGroup/OpenCL-Guide/blob/main/chapters/programming_opencl_kernels.md

  // clRetainKernel(kernel)
  // clReleaseKernel(kernel)
  // clRetainEvent(event)
  // clReleaseEvent(event)

  /*
    Event operator() ( const EnqueueArgs& args, Ts... ts) {
        Event event;
        setArgs<0>(std::forward<Ts>(ts)...);
        args.queue_.enqueueNDRangeKernel(
            kernel_,
            args.offset_,
            args.global_,
            args.local_,
            &args.events_,
            &event);
        return event;
    }

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value, cl_int>::type
        setArg(cl_uint index, const T &value)
    {
        return detail::errHandler(
            ::clSetKernelArg(
                object_,
                index,
                detail::KernelArgumentHandler<T>::size(value),
                detail::KernelArgumentHandler<T>::ptr(value)),
            __SET_KERNEL_ARGS_ERR);
    }

    cl_kernel
    clGetKernelInfo()
    clGetKernelArgInfo()
    clGetKernelWorkGroupInfo()

        cl_int err = detail::errHandler(
            ::clEnqueueNDRangeKernel(
                object_, kernel(), (cl_uint) global.dimensions(),
                offset.dimensions() != 0 ? (const size_type*) offset : NULL,
                (const size_type*) global,
                local.dimensions() != 0 ? (const size_type*) local : NULL,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (event != NULL) ? &tmp : NULL),
            __ENQUEUE_NDRANGE_KERNEL_ERR);


    cl_int enqueueWaitForEvents(const vector<Event>& events) const CL_API_SUFFIX__VERSION_1_1_DEPRECATED
    {
        return detail::errHandler(
            ::clEnqueueWaitForEvents(
                object_,
                (cl_uint) events.size(),
                events.size() > 0 ? (const cl_event*) &events.front() : NULL),
            __ENQUEUE_WAIT_FOR_EVENTS_ERR);
    }

    flush()
    finish()
  */

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
