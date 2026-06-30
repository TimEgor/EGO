#pragma once

#include <cstdint>

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2)))
    #include <immintrin.h>
    #define EGO_MATH_SIMD_SSE2 1
#else
    #define EGO_MATH_SIMD_SSE2 0
#endif

namespace ego
{
    namespace math
    {
        namespace simd
        {
            inline constexpr uint32_t NativeComputeVectorElementCount = 4;

            template <typename T>
            struct NativeComputeVectorStorage
            {
                T m_elements[NativeComputeVectorElementCount];
            };

#if EGO_MATH_SIMD_SSE2
            inline constexpr uint32_t Float32x4ElementCount = 4;
            inline constexpr uint32_t Float64x2ElementCount = 2;

            template <>
            struct NativeComputeVectorStorage<float>
            {
                __m128 m_value;
            };

            template <>
            struct NativeComputeVectorStorage<double>
            {
                __m128d m_lowValue;
                __m128d m_highValue;
            };
#endif

            template <typename T>
            NativeComputeVectorStorage<T> MakeNativeComputeVectorStorage(T _x, T _y, T _z, T _w)
            {
                return NativeComputeVectorStorage<T>{{_x, _y, _z, _w}};
            }

            template <typename T>
            T GetNativeComputeVectorElement(const NativeComputeVectorStorage<T>& _values, uint32_t _index)
            {
                return _values.m_elements[_index];
            }

            template <typename T>
            void SetNativeComputeVectorElement(NativeComputeVectorStorage<T>& _values, uint32_t _index, T _value)
            {
                _values.m_elements[_index] = _value;
            }

#if EGO_MATH_SIMD_SSE2
            inline NativeComputeVectorStorage<float> MakeNativeComputeVectorStorage(float _x, float _y, float _z, float _w)
            {
                NativeComputeVectorStorage<float> result;
                result.m_value = _mm_set_ps(_w, _z, _y, _x);

                return result;
            }

            inline NativeComputeVectorStorage<double> MakeNativeComputeVectorStorage(double _x, double _y, double _z, double _w)
            {
                NativeComputeVectorStorage<double> result;
                result.m_lowValue = _mm_set_pd(_y, _x);
                result.m_highValue = _mm_set_pd(_w, _z);

                return result;
            }

            inline float GetNativeComputeVectorElement(const NativeComputeVectorStorage<float>& _values, uint32_t _index)
            {
                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values.m_value);

                return values[_index];
            }

            inline double GetNativeComputeVectorElement(const NativeComputeVectorStorage<double>& _values, uint32_t _index)
            {
                alignas(16) double values[Float64x2ElementCount];
                if (_index < Float64x2ElementCount)
                {
                    _mm_store_pd(values, _values.m_lowValue);
                    return values[_index];
                }

                _mm_store_pd(values, _values.m_highValue);
                return values[_index - Float64x2ElementCount];
            }

            inline void SetNativeComputeVectorElement(NativeComputeVectorStorage<float>& _values, uint32_t _index, float _value)
            {
                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values.m_value);
                values[_index] = _value;
                _values.m_value = _mm_load_ps(values);
            }

            inline void SetNativeComputeVectorElement(NativeComputeVectorStorage<double>& _values, uint32_t _index, double _value)
            {
                alignas(16) double values[Float64x2ElementCount];
                if (_index < Float64x2ElementCount)
                {
                    _mm_store_pd(values, _values.m_lowValue);
                    values[_index] = _value;
                    _values.m_lowValue = _mm_load_pd(values);

                    return;
                }

                _mm_store_pd(values, _values.m_highValue);
                values[_index - Float64x2ElementCount] = _value;
                _values.m_highValue = _mm_load_pd(values);
            }
#endif
        } // namespace simd
    } // namespace math
} // namespace ego
