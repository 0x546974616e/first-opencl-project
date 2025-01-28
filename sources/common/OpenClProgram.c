#include <CL/opencl.h> // Khronos API

#include <assert.h> // assert()
#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // fprintf(), stderr

#include "common/OpenClContext.h" // OpenClContext{}
#include "common/OpenClProgram.h" // OpenClProgram{}
#include "common/helper.h" // IN, OUT, INOUT, TR_FAILED()

///
/// Displays build errors for a given OpenCL program.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `program` is not NULL and initialized.
/// @pre `context` is not NULL and initialized.
/// @post Display errors on stderr.
///
static bool OpenClProgram_DisplayError(IN OpenClContext* opencl, IN cl_program program) {
  assert(opencl != NULL && opencl->device != NULL);
  assert(program != NULL);

  cl_int error;
  cl_build_status status;

  // CL_BUILD_NONE | CL_BUILD_ERROR | CL_BUILD_SUCCESS
  error = clGetProgramBuildInfo(program, opencl->device, CL_PROGRAM_BUILD_STATUS, sizeof(status), &status, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)", error);
    return false;
  }

  if (status != CL_BUILD_ERROR) {
    TR_ERROR("Given OpenCL program has no build error.");
    return false;
  }

  size_t buildErrorSize = 0u;
  error = clGetProgramBuildInfo(program, opencl->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &buildErrorSize);
  if (error != CL_SUCCESS || buildErrorSize == 0u) {
    TR_FAILED("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG, &buildErrorSize)", error);
    return false;
  }

  char* buildError = (char*) malloc(sizeof(char) * buildErrorSize);
  if (buildError == NULL) {
    TR_ERROR("malloc(char * %zu) failed", buildErrorSize);
    return false;
  }

  error = clGetProgramBuildInfo(program, opencl->device, CL_PROGRAM_BUILD_LOG, buildErrorSize, buildError, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG, &buildError)", error);
    free(buildError);
    return false;
  }

  fprintf(stderr,
   "OpenCL Build Error:" LFLF "%*.*s" // min.max
   , (int) buildErrorSize // TODO: Overflow
   , (int) buildErrorSize
   , buildError
  );

  free(buildError);
  return true;
}

cl_program OpenClProgram_Build(
  IN OpenClContext* opencl,
  IN char const* options,
  IN char const* source,
  IN size_t size
) {
  assert(opencl != NULL);
  assert(options != NULL);
  assert(source != NULL && size > 0);

  cl_int error;
  cl_program program = clCreateProgramWithSource(opencl->context, 1, &source, &size, &error);
  if (error != CL_SUCCESS || program == NULL) {
    TR_FAILED("clCreateProgramWithSource()", error);
    return NULL;
  }

  error = clBuildProgram(program, 0, NULL, options, NULL, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clBuildProgram()", error);
    OpenClProgram_DisplayError(opencl, program);
    OpenClProgram_Release(program);
    return NULL;
  }

  return program;
}

bool OpenClProgram_Release(IN cl_program program) {
  assert(program != NULL); // Debug mode only.

  if (program != NULL) {
    cl_int error = clReleaseProgram(program);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseProgram()", error);
      return false;
    }
  }

  return true;
}
