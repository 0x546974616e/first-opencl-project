#ifndef TR_HELPER_H
#define TR_HELPER_H

#define IN
#define OUT
#define INOUT

#define LF   "\n"
#define LFLF "\n\n"

#define TAB0
#define TAB1 "  "
#define TAB2 TAB1 TAB1
#define TAB3 TAB1 TAB2
#define TAB4 TAB1 TAB3

#define BOLD(X) "\x1B[1m" X "\x1B[0m"

#define TR_STRINGIFY_HELPER(X) #X
#define TR_STRINGIFY(X) TR_STRINGIFY_HELPER(X)

#define TR_CONCAT_HELPER(A, B) A##B
#define TR_CONCAT2(A, B)                      TR_CONCAT_HELPER(A, B)
#define TR_CONCAT3(A, B, C)                   TR_CONCAT2(TR_CONCAT2(A, B), C)
#define TR_CONCAT4(A, B, C, D)                TR_CONCAT2(TR_CONCAT3(A, B, C), D)
#define TR_CONCAT5(A, B, C, D, E)             TR_CONCAT2(TR_CONCAT4(A, B, C, D), E)
#define TR_CONCAT6(A, B, C, D, E, F)          TR_CONCAT2(TR_CONCAT5(A, B, C, D, E), F)
#define TR_CONCAT7(A, B, C, D, E, F, G)       TR_CONCAT2(TR_CONCAT6(A, B, C, D, E, F), G)
#define TR_CONCAT8(A, B, C, D, E, F, G, H)    TR_CONCAT2(TR_CONCAT7(A, B, C, D, E, F, G), H)
#define TR_CONCAT9(A, B, C, D, E, F, G, H, I) TR_CONCAT2(TR_CONCAT8(A, B, C, D, E, F, G, H), I)

#define TR_JOIN1(s, A)                        A
#define TR_JOIN2(s, A, B)          TR_CONCAT3(A, s, B)
#define TR_JOIN3(s, A, B, C)       TR_CONCAT5(A, s, B, s, C)
#define TR_JOIN4(s, A, B, C, D)    TR_CONCAT7(A, s, B, s, C, s, D)
#define TR_JOIN5(s, A, B, C, D, E) TR_CONCAT9(A, s, B, s, C, s, D, s, E)

// __FILE_NAME__ is not standard.
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html

#define TR_FPRINTFN(FILE, FORMAT, ...)                              \
  fprintf(FILE,                                                     \
    __FILE_NAME__ ":%s():" TR_STRINGIFY(__LINE__) ": " FORMAT "\n", \
    __func__, ##__VA_ARGS__                                         \
  )

#define TR_PRINT(FORMAT, ...) TR_FPRINTFN(stdout, FORMAT, ##__VA_ARGS__)
#define TR_ERROR(FORMAT, ...) TR_FPRINTFN(stderr, FORMAT, ##__VA_ARGS__)
#define TR_FAILED(WHAT, ERROR) TR_ERROR(WHAT " failed with 0x%X (%i).", ERROR, ERROR)

#define UNUSED __attribute__((unused)) // [[maybe_unused]]

#endif // TR_HELPER_H
