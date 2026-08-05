#pragma once

#include <cstdint>

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE__) || defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) ||              \
                                           defined(_M_AVX) || defined(_M_AVX512) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1)))
    #include <immintrin.h>
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__AVX512F__) || defined(_M_AVX512))
    #define EGO_MATH_SIMD_AVX512 1
#else
    #define EGO_MATH_SIMD_AVX512 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__AVX2__) || (defined(_M_AVX) && (_M_AVX >= 2)))
    #define EGO_MATH_SIMD_AVX2 1
#else
    #define EGO_MATH_SIMD_AVX2 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__AVX__) || defined(_M_AVX) || EGO_MATH_SIMD_AVX2 || EGO_MATH_SIMD_AVX512)
    #define EGO_MATH_SIMD_AVX 1
#else
    #define EGO_MATH_SIMD_AVX 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE4_2__) || EGO_MATH_SIMD_AVX)
    #define EGO_MATH_SIMD_SSE4_2 1
#else
    #define EGO_MATH_SIMD_SSE4_2 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE4_1__) || EGO_MATH_SIMD_SSE4_2)
    #define EGO_MATH_SIMD_SSE4_1 1
#else
    #define EGO_MATH_SIMD_SSE4_1 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSSE3__) || EGO_MATH_SIMD_SSE4_1)
    #define EGO_MATH_SIMD_SSSE3 1
#else
    #define EGO_MATH_SIMD_SSSE3 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE3__) || EGO_MATH_SIMD_SSSE3)
    #define EGO_MATH_SIMD_SSE3 1
#else
    #define EGO_MATH_SIMD_SSE3 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2)) || EGO_MATH_SIMD_SSE3)
    #define EGO_MATH_SIMD_SSE2 1
#else
    #define EGO_MATH_SIMD_SSE2 0
#endif

#if !defined(EGO_MATH_DISABLE_SIMD) && (defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1)) || EGO_MATH_SIMD_SSE2)
    #define EGO_MATH_SIMD_SSE 1
#else
    #define EGO_MATH_SIMD_SSE 0
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

#if EGO_MATH_SIMD_SSE
            inline constexpr uint32_t Float32x4ElementCount = 4;

            template <>
            struct NativeComputeVectorStorage<float>
            {
                __m128 m_value;
            };
#endif

#if EGO_MATH_SIMD_SSE2
            inline constexpr uint32_t Float64x2ElementCount = 2;

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

#if EGO_MATH_SIMD_SSE
            inline NativeComputeVectorStorage<float> MakeNativeComputeVectorStorage(float _x, float _y, float _z, float _w)
            {
                NativeComputeVectorStorage<float> result;
                result.m_value = _mm_set_ps(_w, _z, _y, _x);

                return result;
            }
#endif

#if EGO_MATH_SIMD_SSE2
            inline NativeComputeVectorStorage<double> MakeNativeComputeVectorStorage(double _x, double _y, double _z, double _w)
            {
                NativeComputeVectorStorage<double> result;
                result.m_lowValue = _mm_set_pd(_y, _x);
                result.m_highValue = _mm_set_pd(_w, _z);

                return result;
            }
#endif

#if EGO_MATH_SIMD_SSE
            inline float GetNativeComputeVectorElement(const NativeComputeVectorStorage<float>& _values, uint32_t _index)
            {
                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values.m_value);

                return values[_index];
            }
#endif

#if EGO_MATH_SIMD_SSE2
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
#endif

#if EGO_MATH_SIMD_SSE
            inline void SetNativeComputeVectorElement(NativeComputeVectorStorage<float>& _values, uint32_t _index, float _value)
            {
                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values.m_value);
                values[_index] = _value;
                _values.m_value = _mm_load_ps(values);
            }
#endif

#if EGO_MATH_SIMD_SSE2
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
