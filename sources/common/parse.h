#ifndef TR_COMMON_PARSE_H
#define TR_COMMON_PARSE_H

#include <stddef.h> // size_t
#include "common/helper.h" // IN, INOUT, OUT

///
/// Parses the multiplicative suffix while moving the cursor.
/// (Allows no suffix).
///
/// Here the supported multiplicative suffixes:
///   - K  = 1000
///   - Ki = 1024
///   - M  = 1000 * 1000
///   - Mi = 1024 * 1024
///   - G  = 1000 * 1000 * 1000
///   - Gi = 1024 * 1024 * 1024
///
/// @param cursor A pointer to the underlying string.
/// @param result The factor related to the suffix.
///
/// @returns True if a suffix is found, false otherwise.
///
/// @pre cursor != NULL && *cursor != NULL && *cursor is null-terminated.
/// @pre result != NULL.
///
bool ParseSuffix(INOUT char const** cursor, OUT size_t* result);

///
/// Parses the number while moving the cursor.
///
/// The number may be followed by a multiplicative suffix (see ParseSuffix).
///
/// @param cursor A pointer to the underlying string.
/// @param result The parsed number.
///
/// @returns True if a number is found, false otherwise.
///
/// @pre cursor != NULL && *cursor != NULL && *cursor is null-terminated.
/// @pre result != NULL.
///
bool ParseNumber(INOUT char const** cursor, OUT size_t* result);

///
/// Parses all numbers while moving the cursor.
///
/// @param cursor A pointer to the underlying string.
/// @param results The list of numbers with the given `size`.
/// @param size The total number of numbers to found.
///
/// @returns True if all numbers has been found, false otherwise.
///
/// @pre cursor != NULL && *cursor != NULL && *cursor is null-terminated.
/// @pre results != NULL.
///
// TODO: Add a separator parameter (char).
bool ParseNumbers(INOUT char const** cursor, OUT size_t* results, IN size_t size);

#endif // TR_COMMON_PARSE_H
