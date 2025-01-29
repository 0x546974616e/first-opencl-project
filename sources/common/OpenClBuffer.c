#include <CL/opencl.h> // Khronos API

#include <assert.h> // assert()
#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // fprintf(), stderr

#include "common/OpenClBuffer.h" // OpenClBuffer{}
#include "common/OpenClContext.h" // OpenClContext{}
#include "common/helper.h" // IN, OUT, INOUT, TR_FAILED(), HAS_ONE_BIT_SET()

// NOTE (DRAFT):
//
// https://stackoverflow.com/questions/57854782/how-opencl-memory-transfer-functions-work
// https://stackoverflow.com/questions/26517114/how-to-compile-opencl-project-with-kernels
//
// https://www.intel.com/content/www/us/en/developer/articles/training/getting-the-most-from-opencl-12-how-to-increase-performance-by-minimizing-buffer-copies-on-intel-processor-graphics.html
// https://www.intel.com/content/www/us/en/docs/opencl-sdk/developer-guide-core-xeon/2018/mapping-memory-objects-use-host-ptr.html
//
// If you already have the data and want to load the data into an OpenCL buffer
// object, then use CL_MEM_USE_HOST_PTR with a buffer allocated at a 4096 byte
// boundary (aligned to a page and cache line boundary) and a total size that is
// a multiple of 64 bytes (cache line size).
//
// error = clGetDeviceInfo(
//   opencl->device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
//   sizeof(cacheLine), &cacheLine, NULL
// );
//
// If your application uses a specific memory management algorithm, or if you
// need more control over memory allocation, you can allocate a buffer and
// then pass the pointer at clCreateBuffer time with the CL_MEM_USE_HOST_PTR
// flag. However, the pointer must be aligned to the certain boundary.
// Otherwise, the framework may perform memory copies. Consider to query the
// required memory alignment using clGetDeviceInfo with
// CL_DEVICE_MEM_BASE_ADDR_ALIGN token.
//
// One may notices that the address you are validating to be on a 4096 byte page
// boundary is a virtual address boundary and not a physical address. Is this
// a problem? While in theory it could be an issue, after all the OS could
// have mapped a virtual address to any physical address, this does not happen
// in our implementation. You are assured that if the virtual address is page
// aligned, then the physical address is page aligned. More details on how and
// why this works is beyond the scope of this article.

cl_mem OpenClBuffer_Create(
  IN cl_context context,
  IN cl_mem_flags flags,
  IN size_t size
) {
  assert(context != NULL);
  assert(HAS_ONE_BIT_SET(CL_MEM_USE_HOST_PTR));
  assert(HAS_ONE_BIT_SET(CL_MEM_COPY_HOST_PTR));
  assert(size > 0);

  cl_int error;
  // Make sure that host pointer is not used.
  flags &= ~(cl_mem_flags) CL_MEM_USE_HOST_PTR;
  flags &= ~(cl_mem_flags) CL_MEM_COPY_HOST_PTR;
  cl_mem buffer = clCreateBuffer(context, flags, size, NULL, &error);
  if (error != CL_SUCCESS) {
    TR_FAILED("clCreateBuffer()", error);
  }

  return buffer;
}

bool OpenClBuffer_Release(IN cl_mem buffer) {
  assert(buffer != NULL);

  cl_int error = clReleaseMemObject(buffer);
  if (error != CL_SUCCESS) {
    TR_FAILED("clReleaseMemObject()", error);
  }

  return error == CL_SUCCESS;
}

bool OpenClBuffer_Read(
  IN cl_command_queue queue,
  IN cl_mem buffer,
  IN void* pointer,
  IN size_t size
) {
  assert(queue != NULL);
  assert(buffer != NULL);
  assert(pointer != NULL);
  assert(size > 0);

  cl_int error;
  size_t actualSize;
  error = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size_t), &actualSize, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetMemObjectInfo(CL_MEM_SIZE)", error);
    return false;
  }

  size = MIN(size, actualSize);
  // TODO: Read or Map/Unmap+Copy?
  error = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0u, size, pointer, 0u, NULL, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clEnqueueReadBuffer()", error);
    return false;
  }

  return true;
}

bool OpenClBuffer_Write(
  IN cl_command_queue queue,
  IN cl_mem buffer,
  IN void* pointer,
  IN size_t size
) {
  assert(queue != NULL);
  assert(buffer != NULL);
  assert(pointer != NULL);
  assert(size > 0);

  cl_int error;
  size_t actualSize;
  error = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size_t), &actualSize, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetMemObjectInfo(CL_MEM_SIZE)", error);
    return false;
  }

  size = MIN(size, actualSize);
  // TODO: Write or Map/Unmap+Copy?
  error = clEnqueueWriteBuffer(queue, buffer, CL_FALSE, 0u, size, pointer, 0u, NULL, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clEnqueueWriteBuffer()", error);
    return false;
  }

  return true;
}
