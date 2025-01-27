#ifndef MATMUL_BLOCKSIZE
#error MATMUL_BLOCKSIZE is undefined.
#endif

#define IN
#define OUT

///
/// ```txt
///                    P
///       N      B B B B            P
///   A A A  *   B B B B  =   C C C C
/// M A A A    N B B B B    M C C C C
/// ```
///
/// @pre get_global_size(0, 1) is (P, M), (x, y) or (columns, rows)
///
__attribute__((reqd_work_group_size(MATMUL_BLOCKSIZE, MATMUL_BLOCKSIZE, 1)))
__kernel void MatMul(
  IN unsigned int const M,
  IN unsigned int const N,
  IN unsigned int const P,

  IN  __global float const* A,
  IN  __global float const* B,
  OUT __global float      * C)
{
  __local float ALocal[MATMUL_BLOCKSIZE][MATMUL_BLOCKSIZE];
  __local float BLocal[MATMUL_BLOCKSIZE][MATMUL_BLOCKSIZE];

  // get_global_size(0) == P
  // get_global_size(1) == M

  size_t xGlobal = get_global_id(0); // [0..P] (Column)
  size_t yGlobal = get_global_id(1); // [0..M] (Row)

  size_t xLocal = get_local_id(0);
  size_t yLocal = get_local_id(1);

  size_t xBlock = get_group_id(0);
  size_t yBlock = get_group_id(1);

  size_t ABase = yBlock * MATMUL_BLOCKSIZE * N;
  size_t BBase = xBlock * MATMUL_BLOCKSIZE;

  size_t AStep = MATMUL_BLOCKSIZE;
  size_t BStep = MATMUL_BLOCKSIZE * P;

  size_t AOffset = yLocal * N + xLocal;
  size_t BOffset = yLocal * P + xLocal;

  float accumulator = 0.0f;

  size_t numberOfBlocks = N / MATMUL_BLOCKSIZE;

  for (size_t nBlock = 0; nBlock < numberOfBlocks; ++nBlock) {

    ALocal[yLocal][xLocal] = A[ABase + AOffset];
    BLocal[xLocal][yLocal] = B[BBase + BOffset]; // Transpose.

    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll
    for (size_t nLocal = 0; nLocal < MATMUL_BLOCKSIZE; ++nLocal) {
      accumulator += ALocal[yLocal][nLocal] * BLocal[xLocal][nLocal];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    ABase += AStep;
    BBase += BStep;
  }

  C[yGlobal * P + xGlobal] = accumulator;
}
