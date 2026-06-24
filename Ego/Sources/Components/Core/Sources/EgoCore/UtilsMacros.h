#pragma once

#include "cstdint"

#define EGO_TO_STRING(_S) #_S
#define EGO_TO_STRING_DEF(_DEF) EGO_TO_STRING(_DEF)

#define EGO_CONCAT(_A, _B) _A##_B
#define EGO_CONCAT_DEF(_A, _B) EGO_CONCAT(_A, _B)

#if defined(_MSC_VER)
    #define EGO_FUNCTION_NAME __FUNCTION__
    #define EGO_FUNCTION_SIGNATURE_NAME __FUNCSIG__
    #define EGO_FILE __FILE__
    #define EGO_LINE __LINE__
    #define EGO_COUNTER __COUNTER__
#else
    #define EGO_FUNCTION_NAME __func__
    #define EGO_FUNCTION_SIGNATURE_NAME __PRETTY_FUNCTION__
    #define EGO_FILE __FILE__
    #define EGO_LINE __LINE__
    #define EGO_COUNTER __COUNTER__
#endif

#define EGO_CHECK_CALL(_CHECK_VAL, _FUNCTION)                                                                                                                                      \
    if (!(_CHECK_VAL))                                                                                                                                                             \
    {                                                                                                                                                                              \
        _FUNCTION;                                                                                                                                                                 \
    }
#define EGO_CHECK_INITIALIZATION(_CHECK_VAL)                                                                                                                                       \
    if (!(_CHECK_VAL))                                                                                                                                                             \
    {                                                                                                                                                                              \
        release();                                                                                                                                                                 \
        return false;                                                                                                                                                              \
    }
#define EGO_CHECK_RETURN_VALUE(_CHECK_VAL, _VALUE)                                                                                                                                 \
    if (!(_CHECK_VAL))                                                                                                                                                             \
    {                                                                                                                                                                              \
        return _VALUE;                                                                                                                                                             \
    }
#define EGO_CHECK_RETURN(_CHECK_VAL)                                                                                                                                               \
    if (!(_CHECK_VAL))                                                                                                                                                             \
    {                                                                                                                                                                              \
        return;                                                                                                                                                                    \
    }
#define EGO_CHECK_RETURN_FALSE(_CHECK_VAL) EGO_CHECK_RETURN_VALUE(_CHECK_VAL, false)
#define EGO_CHECK_RETURN_NULL(_CHECK_VAL) EGO_CHECK_RETURN_VALUE(_CHECK_VAL, nullptr)

#define EGO_SAFE_DESTROY(_OBJ)                                                                                                                                                     \
    {                                                                                                                                                                              \
        if (_OBJ)                                                                                                                                                                  \
        {                                                                                                                                                                          \
            delete _OBJ;                                                                                                                                                           \
            _OBJ = nullptr;                                                                                                                                                        \
        }                                                                                                                                                                          \
    }
#define EGO_SAFE_DESTROY_ARRAY(_OBJ)                                                                                                                                               \
    {                                                                                                                                                                              \
        if (_OBJ)                                                                                                                                                                  \
        {                                                                                                                                                                          \
            delete[] _OBJ;                                                                                                                                                         \
            _OBJ = nullptr;                                                                                                                                                        \
        }                                                                                                                                                                          \
    }
#define EGO_SAFE_DESTROY_WITH_RELEASING(_OBJ)                                                                                                                                      \
    {                                                                                                                                                                              \
        if (_OBJ)                                                                                                                                                                  \
        {                                                                                                                                                                          \
            _OBJ->release();                                                                                                                                                       \
            delete _OBJ;                                                                                                                                                           \
            _OBJ = nullptr;                                                                                                                                                        \
        }                                                                                                                                                                          \
    }
#define EGO_SAFE_RESET_POINTER_WITH_RELEASING(_OBJ)                                                                                                                                \
    {                                                                                                                                                                              \
        if (_OBJ)                                                                                                                                                                  \
        {                                                                                                                                                                          \
            _OBJ->release();                                                                                                                                                       \
            _OBJ.reset();                                                                                                                                                          \
        }                                                                                                                                                                          \
    }

#define EGO_SAFE_CALL(_FUNCTION)                                                                                                                                                   \
    {                                                                                                                                                                              \
        if (_FUNCTION)                                                                                                                                                             \
        {                                                                                                                                                                          \
            _FUNCTION();                                                                                                                                                           \
        }                                                                                                                                                                          \
    }
#define EGO_SAFE_CALL_ARGS(_FUNCTION, ...)                                                                                                                                         \
    {                                                                                                                                                                              \
        if (_FUNCTION)                                                                                                                                                             \
        {                                                                                                                                                                          \
            _FUNCTION(__VA_ARGS__);                                                                                                                                                \
        }                                                                                                                                                                          \
    }
