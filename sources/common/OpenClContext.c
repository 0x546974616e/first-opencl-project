#include <CL/opencl.h> // Khronos API

#include <assert.h> // assert()
#include <stdbool.h> // bool, true, false
#include <stddef.h> // size_t
#include <stdio.h> // fprintf(), stderr

#include "common/OpenClContext.h" // OpenClContext{}
#include "common/helper.h" // IN, OUT, INOUT, TAB, LF, TR_FAILED()
#include "common/parse.h" // ParseNumbers()
#include "common/prefix.h" // IsPrefix()

///
/// Checks if the given string contains the given substring.
///
/// @pre `string` and `subString` are not NULL.
///
static bool StringContains(
  IN char const* string, IN size_t stringLength,
  IN char const* subString, IN size_t subStringLength)
{
  assert(string != 0u && subString != NULL);

  size_t stringIndex = 0u;
  size_t subStringIndex = 0u;

  for (; stringIndex < stringLength && subStringIndex < subStringLength; ++stringIndex) {
    subStringIndex += string[stringIndex] == subString[subStringIndex] ? 1u : -subStringIndex;
    if(subStringIndex == subStringLength) return true;
  }

  return false;
}

bool OpenClContext_FromDeviceType(IN cl_device_type type, OUT OpenClContext* output) {
  assert(output != NULL);

  cl_int error;
  bool success = false;
  cl_platform_id platform = NULL;
  cl_platform_id *platforms = NULL;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;

  // ╔═╗┬  ┌─┐┌┬┐┌─┐┌─┐┬─┐┌┬┐
  // ╠═╝│  ├─┤ │ ├┤ │ │├┬┘│││
  // ╩  ┴─┘┴ ┴ ┴ └  └─┘┴└─┴ ┴

  cl_uint platformCount = 0u;
  error = clGetPlatformIDs(0u, NULL, &platformCount);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetPlatformIDs(&platformCount)", error);
    goto outPlatforms;
  }

  if (platformCount > 0u) {
    // If malloc returns NULL then clGetPlatformIDs() will return CL_INVALID_VALUE.
    // Otherwise, platforms pointer is released in the epilog (goto outPlatforms).
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    error = clGetPlatformIDs(platformCount, platforms, NULL);
    if (error != CL_SUCCESS || platforms == NULL) {
      TR_FAILED("clGetPlatformIDs(&platforms)", error);
      goto outPlatforms;
    }
  }

  cl_uint deviceCount = 0u;
  for (cl_uint index = 0u; index < platformCount && platforms; ++index) {
    error = clGetDeviceIDs(platforms[index], type, 0u, NULL, &deviceCount);
    if (error != CL_SUCCESS && error != CL_DEVICE_NOT_FOUND) {
      TR_FAILED("clGetDeviceIDs(&deviceCount)", error);
      goto outPlatforms;
    }

    if (error != CL_DEVICE_NOT_FOUND && deviceCount > 0u) {
      platform = platforms[index];
      break;
    }
  }

  if (platform == NULL) {
    TR_ERROR("No platform found with required device type.");
    goto outPlatforms;
  }

  // ╔═╗┌─┐┌┐┌┌┬┐┌─┐─┐┬┌┬┐
  // ║  │ ││││ │ ├┤ ┌┼┘ │
  // ╚═╝└─┘┘└┘ ┴ └─┘┴└─ ┴

  // clCreateContextFromType() performs an implicit retain.
  cl_context_properties contextProperties[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0u };
  context = clCreateContextFromType(contextProperties, type, NULL, NULL, &error);
  if (context == NULL || error != CL_SUCCESS) {
    TR_FAILED("clCreateContextFromType()", error);
    goto outContext;
  }

  // ╔╦╗┌─┐┬ ┬┬┌─┐┌─┐
  //  ║║├┤ │┌┘││  ├┤
  // ═╩╝└─┘└┘ ┴└─┘└─┘

  // Given context should have only one device attached (otherwise CL_INVALID_VALUE is returned).
  error = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &device, NULL);
  if (device == NULL || error != CL_SUCCESS) {
    TR_FAILED("clGetContextInfo(CL_CONTEXT_DEVICES)", error);
    goto outDevice;
  }

  // ╔═╗ ┬ ┬┌─┐┬ ┬┌─┐
  // ║═╬╗│ │├┤ │ │├┤
  // ╚═╝╚└─┘└─┘└─┘└─┘

  // Provided by CL_VERSION_2_0 (i.e. available for OpenCL 2.0 or above)
  cl_queue_properties queueProperties[3] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0u };
  queue = clCreateCommandQueueWithProperties(context, device, queueProperties, &error);
  // queue = clCreateCommandQueue(context, device, cl_command_queue_properties)
  if (error != CL_SUCCESS || queue == NULL) {
    TR_FAILED("clCreateCommandQueueWithProperties()", error);
    goto outQueue;
  }

  // Everything is fine here.
  success = true;
  goto out;

outQueue:
  if (queue != NULL) {
    error = clReleaseCommandQueue(queue);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseCommandQueue()", error);
    }
  }

outDevice:
  if (context != NULL) {
    cl_int error = clReleaseContext(context);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseContext()", error);
    }
  }

out:
outContext:
outPlatforms:
  if (platforms != NULL) {
    free(platforms);
  }

  if (success) {
    output->context = context;
    output->platform = platform;
    output->device = device;
    output->queue = queue;
    output->fp64Extension = false;
  }
  else {
    output->context = NULL;
    output->platform = NULL;
    output->device = NULL;
    output->queue = NULL;
    output->fp64Extension = false;
  }

  return success;
}

bool OpenClContext_FromIndexes(IN size_t platformIndex, IN size_t deviceIndex, OUT OpenClContext* output) {
  assert(output != NULL);

  bool success = false;
  cl_platform_id platform, *platforms = NULL;
  cl_device_id device, *devices = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;

  // ╔═╗┬  ┌─┐┌┬┐┌─┐┌─┐┬─┐┌┬┐
  // ╠═╝│  ├─┤ │ ├┤ │ │├┬┘│││
  // ╩  ┴─┘┴ ┴ ┴ └  └─┘┴└─┴ ┴

  cl_uint platformCount = 0u;
  cl_int error = clGetPlatformIDs(0u, NULL, &platformCount);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetPlatformIDs(&platformCount)", error);
    goto outPlatforms;
  }

  if (platformIndex >= platformCount) {
    TR_ERROR("Request for platform[%zu] but %u platforms were found.",
      platformIndex, platformCount);
    goto outPlatforms;
  }

  // If malloc returns NULL then clGetPlatformIDs() will return CL_INVALID_VALUE.
  platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
  error = clGetPlatformIDs(platformCount, platforms, NULL);
  if (error != CL_SUCCESS || platforms == NULL || platforms[platformIndex] == NULL) {
    TR_FAILED("clGetPlatformIDs(&platforms)", error);
    goto outPlatforms;
  }

  // ╔╦╗┌─┐┬ ┬┬┌─┐┌─┐
  //  ║║├┤ │┌┘││  ├┤
  // ═╩╝└─┘└┘ ┴└─┘└─┘

  cl_uint deviceCount = 0u;
  platform = platforms[platformIndex];
  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0u, NULL, &deviceCount);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetDeviceIDs(&deviceCount)", error);
    goto outDevices;
  }

  if (deviceIndex >= deviceCount) {
    TR_ERROR("Request for device[%zu] of platform[%zu] but %u devices were found.",
      deviceIndex, platformIndex, deviceCount);
    goto outDevices;
  }

  // If malloc returns NULL then clGetDeviceIDs() will return CL_INVALID_VALUE.
  devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
  if (error != CL_SUCCESS || devices == NULL || devices[deviceIndex] == NULL) {
    TR_FAILED("clGetDeviceIDs(&devices)", error);
    goto outDevices;
  }

  // ╔═╗┌─┐┌┐┌┌┬┐┌─┐─┐┬┌┬┐
  // ║  │ ││││ │ ├┤ ┌┼┘ │
  // ╚═╝└─┘┘└┘ ┴ └─┘┴└─ ┴

  device = devices[deviceIndex];
  cl_context_properties contextProperties[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0u };
  context = clCreateContext(contextProperties, 1, &device, NULL, NULL, &error);
  if (error != CL_SUCCESS || context == NULL) {
    TR_FAILED("clCreateContext()", error);
    goto outContext;
  }

  // ╔═╗ ┬ ┬┌─┐┬ ┬┌─┐
  // ║═╬╗│ │├┤ │ │├┤
  // ╚═╝╚└─┘└─┘└─┘└─┘

  // Provided by CL_VERSION_2_0 (i.e. available for OpenCL 2.0 or above)
  cl_queue_properties queueProperties[3] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0u };
  queue = clCreateCommandQueueWithProperties(context, device, queueProperties, &error);
  // queue = clCreateCommandQueue(context, device, cl_command_queue_properties)
  if (error != CL_SUCCESS || queue == NULL) {
    TR_FAILED("clCreateCommandQueueWithProperties()", error);
    goto outQueue;
  }

  // Everything is fine now.
  success = true;
  goto out;

outQueue:
  if (queue != NULL) {
    error = clReleaseCommandQueue(queue);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseCommandQueue()", error);
    }
  }

outContext:
  if (context != NULL) {
    error = clReleaseContext(context);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseContext()", error);
    }
  }

out:
outDevices:
  if (devices != NULL) {
    free(devices);
  }

outPlatforms:
  if (platforms != NULL) {
    free(platforms);
  }

  if (success) {
    output->context = context;
    output->platform = platform;
    output->device = device;
    output->queue = queue;
    output->fp64Extension = false;
  }
  else {
    output->context = NULL;
    output->platform = NULL;
    output->device = NULL;
    output->queue = NULL;
    output->fp64Extension = false;
  }

  return success;
}

int OpenClContext_FromString(IN char const* option, OUT OpenClContext* context) {
  assert(context != NULL);
  assert(option != NULL);

  if (IsPrefix(option, "GPU", 4)) {
    return OpenClContext_FromDeviceType(CL_DEVICE_TYPE_GPU, context);
  }

  if (IsPrefix(option, "CPU", 4)) {
    return OpenClContext_FromDeviceType(CL_DEVICE_TYPE_CPU, context);
  }

  if (IsPrefix(option, "Default", 8)) {
    // Or OpenClContext_FromIndexes(0u, 0u)?
    return OpenClContext_FromDeviceType(CL_DEVICE_TYPE_DEFAULT, context);
  }

  size_t indexes[2] = { 0u, 0u };
  char const* cursor = option;
  if (ParseNumbers(&cursor, indexes, 2)) {
    return OpenClContext_FromIndexes(indexes[0], indexes[1], context);
  }

  // TR_ERROR("Unkown option \"%s\"", option);
  return 2;
}

bool OpenClContext_Release(INOUT OpenClContext* this) {
  assert(this != NULL && this->context != NULL && this->queue != NULL);
  // Note thaht OpenCL device and platform should not be released.
  bool success = true;
  cl_int error;

  if (this->queue != NULL) {
    error = clReleaseCommandQueue(this->queue);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseCommandQueue()", error);
      success = false;
    }
  }

  if (this->context != NULL) {
    error = clReleaseContext(this->context);
    if (error != CL_SUCCESS) {
      TR_FAILED("clReleaseContext()", error);
      success = false;
    }
  }

  this->context = NULL;
  this->platform = NULL;
  this->device = NULL;
  this->queue = NULL;

  return success;
}

bool OpenClContext_EnableDoublePrecision(INOUT OpenClContext* this) {
  assert(this != NULL);

  assert(this->context != NULL);
  assert(this->platform != NULL);
  assert(this->device != NULL);

  cl_int error;
  this->fp64Extension = false;

  // ╔═╗┌─┐┌─┐┌─┐┌┐ ┬┬  ┬┌┬┐┬┌─┐┌─┐
  // ║  ├─┤├─┘├─┤├┴┐││  │ │ │├┤ └─┐
  // ╚═╝┴ ┴┴  ┴ ┴└─┘┴┴─┘┴ ┴ ┴└─┘└─┘
  // (First Check)

  cl_device_fp_config config;
  error = clGetDeviceInfo(this->device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config), &config, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetDeviceInfo(CL_DEVICE_DOUBLE_FP_CONFIG)", error);
    return false;
  }

  // According to the specification:
  // If double-precision is supported by the device, then the minimum double-
  // precision floating-point capability must be:
  cl_device_fp_config capabilities =
    CL_FP_FMA              |
    CL_FP_ROUND_TO_NEAREST |
    CL_FP_ROUND_TO_ZERO    |
    CL_FP_ROUND_TO_INF     |
    CL_FP_INF_NAN          |
    CL_FP_DENORM           ;

  if ((config & capabilities) != capabilities) {
    return false;
  }

  // ╔═╗─┐┬┌┬┐┌─┐┌┐┌┌─┐┬┌─┐┌┐┌┌─┐
  // ║╣ ┌┼┘ │ ├┤ │││└─┐││ ││││└─┐
  // ╚═╝┴└─ ┴ └─┘┘└┘└─┘┴└─┘┘└┘└─┘
  // (Double Check)

  size_t extensionsLength = 0u;
  error = clGetPlatformInfo(this->platform, CL_PLATFORM_EXTENSIONS, 0u, NULL, &extensionsLength);
  if (extensionsLength == 0u || error != CL_SUCCESS) {
    TR_FAILED("clGetPlatformInfo(CL_PLATFORM_EXTENSIONS, &length)", error);
    return false;
  }

  char* extensions = (char*) malloc(extensionsLength);
  error = clGetPlatformInfo(this->platform, CL_PLATFORM_EXTENSIONS, extensionsLength, extensions, NULL);
  if (error != CL_SUCCESS || extensions == NULL) {
    TR_FAILED("clGetPlatformInfo(CL_PLATFORM_EXTENSIONS, &extensions)", error);
    if (extensions != NULL) free(extensions);
    return false;
  }

  #define TR_EXTENSIONS(X) StringContains(extensions, extensionsLength, X, sizeof(X) - 1u)
  this->fp64Extension = TR_EXTENSIONS("cl_khr_fp64") || TR_EXTENSIONS("cl_amd_fp64");
  if (extensions != NULL) free(extensions);
  return this->fp64Extension;
}

bool OpenClContext_DisplayInformations(IN OpenClContext* this) {
  assert(this != NULL);

  assert(this->context != NULL);
  assert(this->platform != NULL);
  assert(this->device != NULL);

  cl_int error;

  #define TR_BUFFER_SIZE 1024
  char buffer[TR_BUFFER_SIZE] = { 0u };

  // ╔═╗┬  ┌─┐┌┬┐┌─┐┌─┐┬─┐┌┬┐
  // ╠═╝│  ├─┤ │ ├┤ │ │├┬┘│││
  // ╩  ┴─┘┴ ┴ ┴ └  └─┘┴└─┴ ┴

  printf(LF TAB0 "OpenCL Platform:" LF);

  #define TR_PLATFORM_NAME    TAB1 "Name....: "
  #define TR_PLATFORM_VENDOR  TAB1 "Vendor..: "
  #define TR_PLATFORM_VERSION TAB1 "Version.: "
  #define TR_PLATFORM_ERROR   TAB1 "clGetPlatformInfo() failed"

  error = clGetPlatformInfo(this->platform, CL_PLATFORM_NAME, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_PLATFORM_NAME "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_PLATFORM_ERROR : buffer);

  error = clGetPlatformInfo(this->platform, CL_PLATFORM_VENDOR, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_PLATFORM_VENDOR "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_PLATFORM_ERROR : buffer);

  error = clGetPlatformInfo(this->platform, CL_PLATFORM_VERSION, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_PLATFORM_VERSION "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_PLATFORM_ERROR : buffer);

  // ╔╦╗┌─┐┬ ┬┬┌─┐┌─┐
  //  ║║├┤ │┌┘││  ├┤
  // ═╩╝└─┘└┘ ┴└─┘└─┘

  printf(LF TAB1 "Backend Device:" LF);

  #define TR_DEVICE_TYPE       TAB2 "Type...........: "
  #define TR_DEVICE_NAME       TAB2 "Name...........: "
  #define TR_DEVICE_VENDOR     TAB2 "Vendor.........: "
  #define TR_DEVICE_VENDOR_ID  TAB2 "Vendor.ID......: "
  #define TR_DEVICE_VERSION    TAB2 "Device.Version.: "
  #define TR_DEVICE_DRIVER     TAB2 "Driver.Version.: "
  #define TR_DEVICE_ENDIANNESS TAB2 "Endianness.....: "
  #define TR_DEVICE_ERROR      TAB2 "clGetDeviceInfo() failed"

  cl_device_type type;
  error = clGetDeviceInfo(this->device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
  printf(TR_DEVICE_TYPE "%s" LF
    , error != CL_SUCCESS                ? TR_DEVICE_ERROR
    : type == CL_DEVICE_TYPE_GPU         ? "GPU"
    : type == CL_DEVICE_TYPE_CPU         ? "CPU"
    : type == CL_DEVICE_TYPE_ACCELERATOR ? "Accelerator"
    : type == CL_DEVICE_TYPE_DEFAULT     ? "Default"
    : type == CL_DEVICE_TYPE_CUSTOM      ? "Custom"
    :                                      "Unknown"
  );

  error = clGetDeviceInfo(this->device, CL_DEVICE_NAME, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_DEVICE_NAME "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_DEVICE_ERROR : buffer);

  error = clGetDeviceInfo(this->device, CL_DEVICE_VENDOR, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_DEVICE_VENDOR "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_DEVICE_ERROR : buffer);

  cl_uint vendorId = 0u;
  error = clGetDeviceInfo(this->device, CL_DEVICE_VENDOR_ID, sizeof(vendorId), &vendorId, NULL);
  if (error != CL_SUCCESS) printf(TR_DEVICE_VENDOR_ID TR_DEVICE_ERROR LF);
  else printf(TR_DEVICE_VENDOR_ID "0x%X (%u)" LF, vendorId, vendorId);

  error = clGetDeviceInfo(this->device, CL_DEVICE_VERSION, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_DEVICE_VERSION "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_DEVICE_ERROR : buffer);

  error = clGetDeviceInfo(this->device, CL_DRIVER_VERSION, TR_BUFFER_SIZE, buffer, NULL);
  printf(TR_DEVICE_DRIVER "%.*s" LF, TR_BUFFER_SIZE, error != CL_SUCCESS ? TR_DEVICE_ERROR : buffer);

  cl_uint bits;
  error = clGetDeviceInfo(this->device, CL_DEVICE_ADDRESS_BITS , sizeof(bits), &bits, NULL);
  if (error != CL_SUCCESS) { bits = 0u; }

  cl_bool endianness;
  error = clGetDeviceInfo(this->device, CL_DEVICE_ENDIAN_LITTLE , sizeof(endianness), &endianness, NULL);
  printf(TR_DEVICE_ENDIANNESS "%s, %u bits" LF
    , error != CL_SUCCESS    ? TR_DEVICE_ERROR
    : endianness == CL_TRUE  ? "Little-Endian"
    : endianness == CL_FALSE ? "Big-Endian"
    :                          "Unknown" // Defensive.
    , bits
  );

  // TODO: Display more informations (see clinfo --raw)
  // - Local memory CL_DEVICE_LOCAL_MEM_SIZE cl_ulong
  // - Global memory
  // - work group
  // - work item

  printf(LF);
  return true;
}

bool OpenClContext_DisplayBuildError(IN cl_program program, IN OpenClContext* this) {
  assert(this != NULL && this->device != NULL);
  assert(program != NULL);

  cl_int error;
  cl_build_status status;

  // CL_BUILD_NONE | CL_BUILD_ERROR | CL_BUILD_SUCCESS
  error = clGetProgramBuildInfo(program, this->device, CL_PROGRAM_BUILD_STATUS, sizeof(status), &status, NULL);
  if (error != CL_SUCCESS) {
    TR_FAILED("clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)", error);
    return false;
  }

  if (status != CL_BUILD_ERROR) {
    TR_ERROR("Given OpenCL program has no build error.");
    return false;
  }

  size_t buildErrorSize = 0u;
  error = clGetProgramBuildInfo(program, this->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &buildErrorSize);
  if (error != CL_SUCCESS || buildErrorSize == 0u) {
    TR_FAILED("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG, &buildErrorSize)", error);
    return false;
  }

  char* buildError = (char*) malloc(sizeof(char) * buildErrorSize);
  if (buildError == NULL) {
    TR_ERROR("malloc(char * %zu) failed", buildErrorSize);
    return false;
  }

  error = clGetProgramBuildInfo(program, this->device, CL_PROGRAM_BUILD_LOG, buildErrorSize, buildError, NULL);
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
