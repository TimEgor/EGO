#pragma once

#include "ComputeNativeStorage.h"
#include "ComputeScalar.h"

#if EGO_MATH_SIMD_AVX512
    #include "ComputeSimdAvx512.h"
#elif EGO_MATH_SIMD_AVX2
    #include "ComputeSimdAvx2.h"
#elif EGO_MATH_SIMD_AVX
    #include "ComputeSimdAvx.h"
#elif EGO_MATH_SIMD_SSE4_2
    #include "ComputeSimdSse42.h"
#elif EGO_MATH_SIMD_SSE4_1
    #include "ComputeSimdSse41.h"
#elif EGO_MATH_SIMD_SSSE3
    #include "ComputeSimdSsse3.h"
#elif EGO_MATH_SIMD_SSE3
    #include "ComputeSimdSse3.h"
#elif EGO_MATH_SIMD_SSE2
    #include "ComputeSimdSse2.h"
#elif EGO_MATH_SIMD_SSE
    #include "ComputeSimdSse.h"
#endif

namespace ego
{
    namespace math
    {
        namespace simd
        {
#if EGO_MATH_SIMD_AVX512
            namespace selected = avx512;
#elif EGO_MATH_SIMD_AVX2
            namespace selected = avx2;
#elif EGO_MATH_SIMD_AVX
            namespace selected = avx;
#elif EGO_MATH_SIMD_SSE4_2
            namespace selected = sse42;
#elif EGO_MATH_SIMD_SSE4_1
            namespace selected = sse41;
#elif EGO_MATH_SIMD_SSSE3
            namespace selected = ssse3;
#elif EGO_MATH_SIMD_SSE3
            namespace selected = sse3;
#elif EGO_MATH_SIMD_SSE2
            namespace selected = sse2;
#elif EGO_MATH_SIMD_SSE
            namespace selected = sse;
#else
            namespace selected = scalar;
#endif

            using selected::AbsVectorElements;
            using selected::AddMatrixElements;
            using selected::AddVectorElements;
            using selected::AreMatrixElementsEqual;
            using selected::AreVectorElementsEqual;
            using selected::AreVectorElementsGreater;
            using selected::AreVectorElementsGreaterOrEqual;
            using selected::AreVectorElementsLess;
            using selected::AreVectorElementsLessOrEqual;
            using selected::CrossVector3Elements;
            using selected::DivideMatrixElements;
            using selected::DivideVectorElements;
            using selected::DotDynamicMatrixRowColumnElements;
            using selected::DotDynamicMatrixRowVectorElements;
            using selected::DotVectorElements;
            using selected::IsAnyVectorElementGreater;
            using selected::IsAnyVectorElementGreaterOrEqual;
            using selected::IsAnyVectorElementLess;
            using selected::IsAnyVectorElementLessOrEqual;
            using selected::MultiplyMatrixElements;
            using selected::MultiplyVectorElements;
            using selected::NegateMatrixElements;
            using selected::NegateVectorElements;
            using selected::ReciprocalVectorElements;
            using selected::SubtractMatrixElements;
            using selected::SubtractVectorElements;
            using selected::TransformMatrix2x2VectorElements;
            using selected::TransformMatrix3x3VectorElements;
            using selected::TransformMatrix4x4VectorElements;
        } // namespace simd
    } // namespace math
} // namespace ego
