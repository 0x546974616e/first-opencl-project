#ifndef TR_COMMON_PREFIX_H
#define TR_COMMON_PREFIX_H

#include <stddef.h> // size_t
#include <strings.h> // strncasecmp()

#include "common/helper.h" // IN

///
/// Determines the length of a fixed-size string.
///
/// Could otherwise be enable by Feature Test Macros (see `man strnlen`).
///
static inline size_t strnlen(IN char const* string, IN size_t length) {
  size_t index = 0u;
  while (index < length && string[index]) ++index;
  return index;
}

///
/// Checks whether or not `prefix` is a prefix of `string`.
///
/// The comparison is case-insensitive.
///
/// @pre prefix is not NULL and null-terminated
/// @pre string is not NULL and null-terminated
///
static bool IsPrefix(IN char const* prefix, IN char const* string, IN size_t maxLength) {
  return 0 == strncasecmp(prefix, string, strnlen(prefix, maxLength));
}

#endif // TR_COMMON_PREFIX_H
