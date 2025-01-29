#ifndef TR_COMMON_PROGRAM_H
#define TR_COMMON_PROGRAM_H

#include <CL/opencl.h> // Khronos API
#include <stdbool.h> // bool, true, false
#include "common/helper.h" // IN, INOUT, OUT
#include "common/OpenClContext.h" // OpenClContext{}

///
/// Loads and compiles an OpenCL program.
///
/// @returns A non-NULL program on success, NULL otherwise.
///
/// @pre `context` is not NULL and already initialized.
/// @pre `options` is not NULL and null-terminated.
/// @pre `source` is not NULL and `size` is non-zero.
/// @post May displays build errors on stderr.
///
cl_program OpenClProgram_Build(
  IN OpenClContext* context,
  IN char const* options,
  IN char const* source,
  IN size_t size
);

///
/// Release an OpenCL program.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `program` is not NULL.
///
bool OpenClProgram_Release(IN cl_program program);

#endif // TR_COMMON_PROGRAM_H
