#include <stdio.h> // FILE, stdout, stderr
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE

#include "common/prefix.h" // IsPrefix()
#include "matrix/MatMulContext.h" // Self{}
#include "matrix/MatMulProgram.h" //

#define TR_COMMAND_MATMUL "matmul"

static void Usage(FILE* stream) {
  MatMulContext_ArgumentsUsage(stream, TR_COMMAND_MATMUL);
}

int main(int argc, char* argv[]) {
  (void) argc; (void) argv;

  if (argc <= 1) {
    Usage(stdout);
    return EXIT_SUCCESS;
  }

  if (IsPrefix(argv[1], TR_COMMAND_MATMUL, sizeof(TR_COMMAND_MATMUL))) {
    argv[1] = TR_COMMAND_MATMUL;

    MatMulContext context;
    int result = MatMulContext_FromArguments(argc - 1, argv + 1, &context);
    if (result == 1) { // 2 is --help
      MatMulContext_Display(&context);
      MatMulProgram_Run(&context);
      MatMulContext_Release(&context);
    }

    return result == 0 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  Usage(stdout);
  return EXIT_SUCCESS;
}

  // https://www.bealto.com/gpu-fft2_real-type.html
  // https://streamhpc.com/blog/2013-10-17/writing-opencl-code-single-double-precision/
  // https://stackoverflow.com/questions/40193346/opencl-double-precision-not-working
