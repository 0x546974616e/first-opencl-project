#ifndef TR_MATRIX_MATMULCONTEXT_H
#define TR_MATRIX_MATMULCONTEXT_H

#include <CL/opencl.h> // Khronos API

#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // FILE

#include "common/OpenClContext.h" // OpenClContext{}
#include "common/helper.h" // IN, INOUT, OUT, TR_PRINT()

#define TR_MATMUL_LOG(CONTEXT, LEVEL, FORMAT, ...) \
  if (LEVEL <= CONTEXT->verbose) { TR_PRINT(FORMAT, ##__VA_ARGS__); }

///
/// Gather all the parameters to run the matrix multiplication.
///
typedef struct MatMulContext {
  OpenClContext openCl;

  /// The block size of the blocked matrix multiplication (must be even).
  size_t blockSize;

  /// Contains the matrix sizes with their padding.
  /// A(M, N) * B(N, P) = C(M, P)
  size_t M, paddingM;
  size_t N, paddingN;
  size_t P, paddingP;

  /// Whether or not to check the matrix multiplication with a naive,
  /// potentially long, CPU implementation.
  bool cpuCheck;

  /// Verbose level.
  size_t verbose;
} MatMulContext;

///
/// Displays command line arguments usage on given stream.
///
/// @pre `stream` is not NULL.
/// @pre `command` is not NULL.
///
bool MatMulContext_ArgumentsUsage(IN FILE* stream, IN char const* command);

///
/// Creates a `MatMulContext` from the command line arguments.
///
/// @returns `2` if `--help` was provided (thus invalidate the context), `1` on
///          success and `0` otherwise.
///
/// @pre `context` is not NULL.
/// @pre `argv` is not NULL and contains at least one null-terminated string.
/// @post May displays error on stderr and help on stdout.
///
int MatMulContext_FromArguments(IN int argc, IN char* argv[], OUT MatMulContext* context);

///
/// Releases the `MatMulContext` resources.
///
/// @pre `context` is not NULL and already initialized.
///
bool MatMulContext_Release(INOUT MatMulContext* context);

///
/// Returns the total waste of elements of matrixes A, B and C (Because of the padding).
///
/// This does not take the floating-point sizes (float or double) into consideration.
///
/// @pre `context` is not NULL and initialized.
///
size_t MatMulContext_ComputeWaste(IN MatMulContext const* this);

///
/// Displays informations about the given context.
///
/// @pre `context` is not NULL and already initialized.
/// @post Displays on stdout.
///
bool MatMulContext_Display(IN MatMulContext* context);

#endif // TR_MATRIX_MATMULCONTEXT_H
