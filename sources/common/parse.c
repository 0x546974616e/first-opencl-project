#include <assert.h> // assert()
#include <ctype.h> // isalpha(), isdigit(), ispunct(), tolower(), toupper()
#include <stddef.h> // size_t

#include "common/helper.h" // IN, INOUT, OUT
#include "common/parse.h" // Self

bool ParseSuffix(INOUT char const** cursor, OUT size_t* result) {
  assert(cursor != NULL && *cursor != NULL);
  assert(result != NULL);

  size_t multiplicator = 1u;
  if (isalpha(**cursor)) {
    // Computes suffix length ('i' means binary factor).
    size_t length = tolower((*cursor)[1]) == 'i' ? 2u : 1u;
    size_t factor = length == 2u ? 1024u : 1000u;

    switch (toupper(**cursor)) {
      case 'G': multiplicator *= factor;
      case 'M': multiplicator *= factor;
      case 'K': multiplicator *= factor;
        *cursor += length;
        break;

      default:
        *result = 0u;
        return false;
    }
  }

  *result = multiplicator;
  return true;
}

bool ParseNumber(INOUT char const** cursor, OUT size_t* result) {
  assert(cursor != NULL && *cursor != NULL);
  assert(result != NULL);

  size_t number = 0u;
  char const* backup = *cursor;
  for (; isdigit(**cursor); *cursor += 1u) {
    number = (number * 10u) + (size_t) (**cursor - '0');
  }

  size_t factor = 0u;
  // A number must have at least one digit.
  if (*cursor > backup && ParseSuffix(cursor, &factor)) {
    *result = number * factor;
    return true;
  }

  *result = 0u;
  return false;
}

bool ParseNumbers(INOUT char const** cursor, OUT size_t* results, IN size_t size) {
  assert(cursor != NULL && *cursor != NULL);
  assert(results != NULL);

  size_t index = 0u;
  // TODO: Trim whitespaces?
  while (index < size && (index == 0 || ispunct(**cursor))) {
    if (index != 0u) *cursor += 1; // Consumes the separator.
    if (ParseNumber(cursor, results + index)) index += 1u; else break;
  }

  // TODO: Consistency with ParseNumber() `**cursor == 0x0`.
  return index == size && **cursor == '\0';
}
