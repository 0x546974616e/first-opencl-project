#ifndef TR_COMMON_BUFFER_H
#define TR_COMMON_BUFFER_H

#include <CL/opencl.h> // Khronos API
#include <stdbool.h> // bool, true, false
#include "common/helper.h" // IN, INOUT, OUT
#include "common/OpenClContext.h" // OpenClContext{}

///
/// Create a new OpenCL Buffer without any host data.
///
/// The given flags can be one of the following value:
/// - `CL_MEM_READ_WRITE`
/// - `CL_MEM_WRITE_ONLY`
/// - `CL_MEM_READ_ONLY`
/// - ...
///
/// @returns A non-NULL OpenCL Memory on success, NULL otherwise.
///
/// @pre `context` is not NULL and initialized.
///
cl_mem OpenClBuffer_Create(
  IN cl_context context,
  IN cl_mem_flags flags,
  IN size_t size
);

///
/// Release a device buffer.
///
/// @returns `true` on success, `false` otherwise.
///
bool OpenClBuffer_Release(IN cl_mem buffer);

///
/// Read from device buffer into host pointer (blocking command).
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `buffer` is not NULL.
/// @pre `pointer` is not NULL.
/// @pre The size of the data that `pointer` points to
///      must be greater than or equal to `size` bytes.
///
bool OpenClBuffer_Read(
  IN cl_command_queue queue,
  IN cl_mem buffer,
  IN void* pointer,
  IN size_t size
);

///
/// Write to device buffer from host pointer (non-blocking command).
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `buffer` is not NULL.
/// @pre `pointer` is not NULL.
/// @pre The size of the data that `pointer` points to
///      must be greater than or equal to `size` bytes.
///
bool OpenClBuffer_Write(
  IN cl_command_queue queue,
  IN cl_mem buffer,
  IN void* pointer,
  IN size_t size
);

#endif // TR_COMMON_BUFFER_H
