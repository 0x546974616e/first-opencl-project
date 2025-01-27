#include <assert.h> // assert()
#include <getopt.h> // getopt_long(), required_argument, no_argument
#include <stdbool.h> // bool, true, false
#include <stdio.h> // FILE, fprintf, stdout, stderr

#include "common/helper.h" // IN, INOUT, OUT, TAB, LF
#include "common/parse.h" // ParseNumbers()
#include "matrix/MatMulContext.h" // MatMulContext{}

#define TR_MATMUL_STRING(TAB) \
  TAB "                   P"              LF \
  TAB "      N      B B B B            P" LF \
  TAB "  A A A  *   B B B B  =   C C C C" LF \
  TAB "M A A A    N B B B B    M C C C C" LF \

///
/// Round `x` number up to `n`.
///
static size_t RoundUp(IN size_t x, IN size_t n) {
  size_t r = x % n;
  return r == 0 ? x : x + n - r;
}

bool MatMulContext_ArgumentsUsage(IN FILE* stream, char const* command) {
  assert(stream != NULL);
  assert(command != NULL);

  fprintf(stream,
    LF TAB1 BOLD("%s") LF

    TAB2 "Multiplies two matrixes together:"  LF
    TR_MATMUL_STRING(TAB3) LF

    TAB2 BOLD("-d, --device") " GPU | CPU | Default | <PlatformIndex>:<DeviceIndex>" LF // TODO: Dedup.
    TAB3 "Specifies which device to use (prefix, case-insensitive)." LFLF

    TAB2 BOLD("-m, --matrix-size") " <M>,<N>,<P>" LF // TODO: Dedup.
    TAB3 "Represents the matrixes sizes with optional multiplicative suffixes (K, Ki, M, Mi...)." LFLF

    TAB2 BOLD("-f, --double-precision") LF
    TAB3 "Enables the double-precision floating-point extension." LFLF

    TAB2 BOLD("-b, --block-size") " <Size>" LF
    TAB3 "The block size of the block-wise matrix multiplication." LFLF

    TAB2 BOLD("-c, --cpu-check") LF
    TAB3 "Checks the OpenCL result with a naive, potentially long, CPU implementation." LFLF

    TAB2 BOLD("-v, --verbose") LF
    TAB3 "Displays more informations (may appear multiple times)." LFLF

    TAB2 BOLD("-h, --help") LF
    TAB3 "Displays this help and quit." LFLF

    , command
  );

  return true;
}

int MatMulContext_FromArguments(IN int argc, IN char* argv[], OUT MatMulContext* this) {
  assert(argc >= 1 && argv[0] != NULL);
  assert(this != NULL);

  static struct option options[] = {
    { "device", required_argument, NULL, 'd' },
    { "block-size", required_argument, NULL, 'b' },
    { "matrix-size", required_argument, NULL, 'm' },
    { "double-precision", no_argument, NULL, 'f' },
    { "cpu-check", no_argument, NULL, 'c' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 },
  };

  int option;
  char const* device = NULL;
  char const* matrixSize = NULL;
  bool doublePrecision = false;
  char const* blockSize = NULL;

  this->blockSize = 16u; // Default.
  this->cpuCheck = false;
  this->verbose = 0u;

  opterr = 1; // Prints error on stderr.
  while (0 <= (option = getopt_long(argc, argv, "d:b:m:fcvh", options, NULL))) {
    switch (option) {
      case 'd': device = optarg; break;
      case 'b': blockSize = optarg; break;
      case 'm': matrixSize = optarg; break;
      case 'c': this->cpuCheck = true; break;
      case 'f': doublePrecision = true; break;
      case 'v': this->verbose += 1u; break;
      case 'h':
        MatMulContext_ArgumentsUsage(stdout, argv[0]);
        return 2;
      // ? : default
    }
  }

  if (blockSize != NULL) {
    size_t size = 0u;
    char const* blockCursor = blockSize;
    // TODO: Even or power of 2?
    if (!ParseNumbers(&blockCursor, &size, 1) || size <= 0 || size % 2 != 0) {
      int padding = blockCursor > blockSize ? (int) (blockCursor - blockSize) + 1 : 0;
      fprintf(stderr, LF
        "The block size for the blocked matrix multiplication must be even." LF
        TAB1 "--block-size %s" LF
        TAB1 "             %*c Unexpected character or value" LFLF
        , blockSize, padding, '^'
      );

      return false;
    }

    this->blockSize = size;
  }

  size_t sizes[3] = { 0u, 0u, 0u };
  char const* matrixCursor = matrixSize;
  if (matrixSize == NULL || !ParseNumbers(&matrixCursor, sizes, 3)) {
    int padding = matrixCursor > matrixSize ? (int) (matrixCursor - matrixSize) + 1 : 0;
    fprintf(stderr, LF
      "Matrix sizes must be a comma-separated list of 3 numbers:" LF
      "(with optional multiplicative suffixes)" LF
      TAB1 "--matrix-size %s" LF
      TAB1 "              %*c Unexpected character" LFLF
      , matrixSize != NULL ? matrixSize : "(empty)", padding, '^'
    );

    return false;
  }

  this->M = sizes[0];
  this->N = sizes[1];
  this->P = sizes[2];

  this->paddingM = RoundUp(this->M, this->blockSize) - this->M;
  this->paddingN = RoundUp(this->N, this->blockSize) - this->N;
  this->paddingP = RoundUp(this->P, this->blockSize) - this->P;

  if (device == NULL) { device = "GPU"; }
  switch (OpenClContext_FromString(device, &this->openCl)) {
    case 1: break; // Ok, true

    case 2:
      fprintf(stderr, LF
        "An invalid OpenCL device option has been found:" LF
        TAB1 "--device %s" LFLF
        "A device must be one of the following values (or prefix, case-insensitive):" LF
        TAB1 "--device GPU | GPU | Default | <PlatformIndex>:<DeviceIndex>" LFLF
        , device
      );

    default:
      return false;
  }

  if (doublePrecision && !OpenClContext_EnableDoublePrecision(&this->openCl)) {
    fprintf(stderr, LF
      "Double-precision floating-point was required but the target platform does not support it." LFLF
    );

    if (!OpenClContext_Release(&this->openCl)) {
      TR_ERROR("OpenClContext_Release() failed");
    }

    return false;
  }

  return true;
}

bool MatMulContext_Release(INOUT MatMulContext* this) {
  assert(this != NULL);
  this->M = this->N = this->P = 0u;
  this->paddingM = this->paddingN = this->paddingP = 0u;
  return OpenClContext_Release(&this->openCl);
}

size_t MatMulContext_ComputeWaste(IN MatMulContext const* this) {
  assert(this != NULL);
  size_t wasteA = (this->paddingM * this->N) + (this->paddingN * this->M) + (this->paddingM * this->paddingN);
  size_t wasteB = (this->paddingN * this->P) + (this->paddingP * this->N) + (this->paddingN * this->paddingP);
  size_t wasteC = (this->paddingM * this->P) + (this->paddingP * this->M) + (this->paddingM * this->paddingP);
  return (wasteA + wasteB + wasteC) * (this->openCl.fp64Extension ? sizeof(double) : sizeof(float));
}

bool MatMulContext_Display(IN MatMulContext* this) {
  assert(this != NULL);

  if (!OpenClContext_DisplayInformations(&this->openCl)) {
    TR_ERROR("OpenClContext_DisplayInformations() failed");
  }

  size_t waste = MatMulContext_ComputeWaste(this);

  printf(
    TAB0 "Matrix Multiplication:" LF

    TAB1 "Block.Size.............: %zu" LF
    TAB1 "M.Dimension.(+padding).: %zu (+%zu)" LF
    TAB1 "N.Dimension.(+padding).: %zu (+%zu)" LF
    TAB1 "P.Dimension.(+padding).: %zu (+%zu)" LF
    TAB1 "Total.Waste............: %zu Byte%c" LF
    TAB1 "Floating-Point.Format..: %s-Precision (%s)" LF
    TAB1 "CPU.Check..............: %s" LF
    TAB1 "Verbose.Level..........: %zu" LFLF

    , this->blockSize
    , this->M, this->paddingM
    , this->N, this->paddingN
    , this->P, this->paddingP
    , waste, waste >= 2 ? 's' : ' '
    , this->openCl.fp64Extension ? "Double" : "Single"
    , this->openCl.fp64Extension ? "double" : "float"
    , this->cpuCheck ? "True" : "False"
    , this->verbose
  );

  if (this->verbose >= 1) {
    printf(TR_MATMUL_STRING(TAB2) LF);
  }

  return true;
}
