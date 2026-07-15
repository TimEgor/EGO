#pragma once

#include "ComputeSimdSsse3.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
            namespace sse41
            {
                using namespace ssse3;

                template <typename VectorType>
                typename VectorType::ValueType DotVectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        const __m128 vector1Values = sse2::LoadFloat32x4(_vector1, _elementCount);
                        const __m128 vector2Values = sse2::LoadFloat32x4(_vector2, _elementCount);

                        switch (_elementCount)
                        {
                            case 2:
                                return _mm_cvtss_f32(_mm_dp_ps(vector1Values, vector2Values, 0x31));
                            case 3:
                                return _mm_cvtss_f32(_mm_dp_ps(vector1Values, vector2Values, 0x71));
                            case 4:
                                return _mm_cvtss_f32(_mm_dp_ps(vector1Values, vector2Values, 0xF1));
                            default:
                                break;
                        }
                    }

                    return ssse3::DotVectorElements(_vector1, _vector2, _elementCount);
                }
            } // namespace sse41
        } // namespace simd
    } // namespace math
} // namespace ego
