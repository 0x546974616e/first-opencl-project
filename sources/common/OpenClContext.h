#ifndef TR_COMMON_CONTEXT_H
#define TR_COMMON_CONTEXT_H

#include <CL/opencl.h> // Khronos API
#include <stdbool.h> // bool, true, false
#include "common/helper.h" // IN, INOUT, OUT

///
/// A `OpenClContext` consists of an OpenCL context with one attached device
/// with its platform and a default queue.
///
typedef struct OpenClContext {
  cl_context context;
  cl_platform_id platform;
  cl_device_id device;
  cl_command_queue queue;

  /// Whether or not the double-precision extension is available.
  /// (Coming from cl_khr_fp64 or cl_amd_fp64)
  bool fp64Extension;
} OpenClContext;

///
/// Creates an `OpenClContext` from an OpenCL device type (e.g. `CL_DEVICE_TYPE_GPU`).
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `context` is not NULL.
/// @post May display error on stderr.
///
bool OpenClContext_FromDeviceType(IN cl_device_type type, OUT OpenClContext* context);

///
/// Creates an `OpenClContext` from a device in the a platform given by their indexes.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `context` is not NULL.
/// @post May display error on stderr.
///
bool OpenClContext_FromIndexes(IN size_t platformIndex, IN size_t deviceIndex, OUT OpenClContext* context);

///
/// Creates a new `OpenClContext` from what describe the given string.
///
/// Given string may contains the following values (or prefix, case-insensitive):
///   - `GPU`, `CPU` or `Default` to get first available required device,
///   - `P:D` where `P` is the platform index and `D` the device index.
///
/// @returns `2` for an unknown option, `1` on success, `0` otherwise.
///
/// @pre `context` is not NULL.
/// @pre `option` is not NULL and null-terminated.
/// @post May display error on stderr.
///
int OpenClContext_FromString(IN char const* option, OUT OpenClContext* context);

///
/// Releases an `OpenClContext`.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `context` is not NULL and already initialized.
/// @post May display error on stderr.
///
bool OpenClContext_Release(INOUT OpenClContext* context);

///
/// Checks if double-precision floating-point extension is available and updates
/// the `OpenClContext` accordingly.
///
/// @returns `true` is avaible, `false` otherwise.
///
/// @pre `context` is not NULL and already initialized.
/// @post May display error on stderr.
///
bool OpenClContext_EnableDoublePrecision(INOUT OpenClContext* context);

///
/// Displays informations about the associated platform and device of the context.
///
/// @returns `true` if everyting went fine, `false` otherwise.
///
/// @pre `context` is not NULL and already initialized.
/// @post May display error on stderr.
/// @post Display content on stdout.
///
bool OpenClContext_DisplayInformations(IN OpenClContext* context);

///
/// Displays build errors for a given OpenCL program.
///
/// @returns `true` on success, `false` otherwise.
///
/// @pre `program` is not NULL and initialized.
/// @pre `context` is not NULL and initialized.
/// @post Display errors on stderr.
///
bool OpenClContext_DisplayBuildError(IN cl_program program, IN OpenClContext* context);

#endif // TR_COMMON_CONTEXT_H
