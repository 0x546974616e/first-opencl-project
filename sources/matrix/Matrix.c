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
#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // fprintf()

#include "common/helper.h" // IN, OUT, INOUT, TR_FAILED(), RoundUp()
#include "matrix/Matrix.h" // Matrix(), Self{}

bool Matrix(NewWithHostMemory)(
  IN size_t rows,    IN size_t rowPadding,
  IN size_t columns, IN size_t columnPadding,
  IN OpenClContext* opencl,
  OUT Matrix()* matrix
) {
  assert(opencl != NULL);
  assert(matrix != NULL);

  (void) rows; (void) columns;
  (void) rowPadding; (void) columnPadding;

  cl_int error;
  cl_uint cacheLine;

  // f your application uses a specific memory management algorithm, or if you
  // need more control over memory allocation, you can allocate a buffer and
  // then pass the pointer at clCreateBuffer time with the CL_MEM_USE_HOST_PTR
  // flag. However, the pointer must be aligned to the certain boundary.
  // Otherwise, the framework may perform memory copies. Consider to query the
  // required memory alignment using clGetDeviceInfo with
  // CL_DEVICE_MEM_BASE_ADDR_ALIGN token.
  // https://www.intel.com/content/www/us/en/docs/opencl-sdk/developer-guide-core-xeon/2018/mapping-memory-objects-use-host-ptr.html

  // At least with Intel, the host pointer must be align with the page size (4096)
  // and the size be a multiple of the cache line size (64).

  // ou may notice that the address you are validating to be on a 4096 byte page
  // boundary is a virtual address boundary and not a physical address. Is this
  // a problem? While in theory it could be an issue, after all the OS could
  // have mapped a virtual address to any physical address, this does not happen
  // in our implementation. You are assured that if the virtual address is page
  // aligned, then the physical address is page aligned. More details on how and
  // why this works is beyond the scope of this article.

  // https://www.intel.com/content/www/us/en/developer/articles/training/getting-the-most-from-opencl-12-how-to-increase-performance-by-minimizing-buffer-copies-on-intel-processor-graphics.html
  error = clGetDeviceInfo(
    opencl->device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
    sizeof(cacheLine), &cacheLine, NULL
  );

  if (error != CL_SUCCESS) {
    TR_FAILED("clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE)", error);
    return false;
  }

  // TODO: Check addition/multiplication overflow.
  size_t totalSize = (rows + rowPadding) * (columns + columnPadding);
  if (cacheLine > 0) { totalSize = RoundUp(totalSize, cacheLine); }

  // GetPageSize()
  // _align_malloc()
  //
  return false;
}

bool Matrix(NewWithDeviceMemory)(
  IN size_t rows,    IN size_t rowPadding,
  IN size_t columns, IN size_t columnPadding,
  IN OpenClContext* context,
  OUT Matrix()* matrix
) {
  (void) matrix;
  (void) context;
  (void) rows; (void) columns;
  (void) rowPadding; (void) columnPadding;
  // Map, ALLOC? COPY?
  return false;
}

bool Matrix(Release)(INOUT Matrix()* this) {
  assert(this != NULL && this->memory != NULL);

  cl_int error;
  cl_mem_flags flags;

  error = clGetMemObjectInfo(this->memory, CL_MEM_FLAGS, sizeof(flags), &flags, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetMemObjectInfo(CL_MEM_FLAGS)", error);
    return false;
  }

  if (flags & CL_MEM_USE_HOST_PTR) {
    // free
  }
  else {
    // unmap
  }

  return true;
}

bool Matrix(RandomInitialization)(
  IN TR_MATRIX_PRECISION min,
  IN TR_MATRIX_PRECISION max,
  INOUT Matrix()* this
) {
  assert(this != NULL && this->pointer != NULL);

  (void) min; (void) max;
  (void) this;

  return false;
}

#endif // TR_MATRIX_PRECISION
#endif // TR_MATRIX_C
