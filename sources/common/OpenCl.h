#ifndef TR_COMMON_OPENCL_H
#define TR_COMMON_OPENCL_H

#include <stddef.h> // size_t

#define TR_SIZE_ARGS_HELPER(_1, _2, _3, _4, _5, _6, N, ...) N
#define TR_SIZE_ARGS(...) TR_SIZE_ARGS_HELPER(__VA_ARGS__ __VA_OPT__(,) 6, 5, 4, 3, 2, 1, 0)

#define TR_CALL(FUNCTION, ...) FUNCTION(__VA_ARGS__)
#define TR_CALL_CONCAT(...)  TR_CALL(TR_CONCAT2(TR_CONCAT, TR_SIZE_ARGS(__VA_ARGS__)),    __VA_ARGS__)
#define TR_CALL_JOIN(s, ...) TR_CALL(TR_CONCAT2(TR_JOIN,   TR_SIZE_ARGS(__VA_ARGS__)), s, __VA_ARGS__)

#define TR_OPENCL_SYMBOL(suffix, ...) TR_JOIN4(_, _binary, TR_CALL_JOIN(_, __VA_ARGS__), cl, suffix)
#define TR_OPENCL_NAME(suffix, ...) TR_CALL_CONCAT(__VA_ARGS__, suffix)

#define TR_OPENCL_IMPORT_HELPER(SYMBOL_START, SYMBOL_END, START, END) \
  extern char const SYMBOL_START[]; \
  extern char const SYMBOL_END[]; \
  static char const* START = SYMBOL_START; \
  static char const* END = SYMBOL_END;

///
/// Imports an OpenCL file.
///
/// Note: Works for at most 6 variadic parameters.
///
/// Example:
///
/// `TR_OPENCL_IMPORT(matrix, MatMul)` will define a string starting with
/// `matrixMatMulStart` and ending with `matrixMatMulEnd` and containing the
/// content of the file `matrix/MatMul.cl`.
///
#define TR_OPENCL_IMPORT(...) TR_OPENCL_IMPORT_HELPER( \
    TR_OPENCL_SYMBOL(start, __VA_ARGS__), \
    TR_OPENCL_SYMBOL(end, __VA_ARGS__), \
    TR_OPENCL_NAME(Start, __VA_ARGS__), \
    TR_OPENCL_NAME(End, __VA_ARGS__) \
  )

#endif // TR_COMMON_OPENCL_H
