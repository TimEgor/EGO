#pragma once

#if EGO_MATH_SIMD_AVX2
    #include "ComputeSimdAvx2.h"
#else
    #include "ComputeSimdAvx.h"
#endif

namespace ego
{
    namespace math
    {
        namespace simd
        {
            namespace avx512
            {
#if EGO_MATH_SIMD_AVX2
                using namespace avx2;
#else
                using namespace avx;
#endif
            } // namespace avx512
        } // namespace simd
    } // namespace math
} // namespace ego
