#pragma once

#include <cassert>

#include "AssertGenerator.h"
#include "EgoCore/Context/DiagnosticContext.h"
#include "EgoCore/UtilsMacros.h"

#ifndef EGO_ENABLE_ASSERTS
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE)
        #define EGO_ENABLE_ASSERTS 1
    #else
        #define EGO_ENABLE_ASSERTS 0
    #endif
#endif

#if EGO_ENABLE_ASSERTS
    #define EGO_ASSERT(_CONDITION)                                                                                                                                                 \
        if (!(_CONDITION))                                                                                                                                                         \
        {                                                                                                                                                                          \
            ego::AssertGeneratorPointer generator = ego::context::GetAssertGenerator();                                                                                            \
            if (generator)                                                                                                                                                         \
            {                                                                                                                                                                      \
                generator->generateError(EGO_TO_STRING(_CONDITION), EGO_FILE, EGO_LINE);                                                                                           \
            }                                                                                                                                                                      \
            else                                                                                                                                                                   \
            {                                                                                                                                                                      \
                assert(_CONDITION);                                                                                                                                                \
            }                                                                                                                                                                      \
        }

    #define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE)                                                                                                                               \
        if (!(_CONDITION))                                                                                                                                                         \
        {                                                                                                                                                                          \
            ego::AssertGeneratorPointer generator = ego::context::GetAssertGenerator();                                                                                            \
            if (generator)                                                                                                                                                         \
            {                                                                                                                                                                      \
                generator->generateError(_MESSAGE, EGO_FILE, EGO_LINE);                                                                                                            \
            }                                                                                                                                                                      \
            else                                                                                                                                                                   \
            {                                                                                                                                                                      \
                assert((_CONDITION) && (_MESSAGE));                                                                                                                                \
            }                                                                                                                                                                      \
        }

    #define EGO_ASSERT_FAIL()                                                                                                                                                      \
        ego::AssertGeneratorPointer generator = ego::context::GetAssertGenerator();                                                                                                \
        if (generator)                                                                                                                                                             \
        {                                                                                                                                                                          \
            generator->generateError("FAIL", EGO_FILE, EGO_LINE);                                                                                                                  \
        }                                                                                                                                                                          \
        else                                                                                                                                                                       \
        {                                                                                                                                                                          \
            assert(false);                                                                                                                                                         \
        }

    #define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE)                                                                                                                                      \
        ego::AssertGeneratorPointer generator = ego::context::GetAssertGenerator();                                                                                                \
        if (generator)                                                                                                                                                             \
        {                                                                                                                                                                          \
            generator->generateError(_MESSAGE, EGO_FILE, EGO_LINE);                                                                                                                \
        }                                                                                                                                                                          \
        else                                                                                                                                                                       \
        {                                                                                                                                                                          \
            assert(false && (_MESSAGE));                                                                                                                                           \
        }

#else
    #define EGO_ASSERT(_CONDITION) ((void)0)
    #define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE) ((void)0)
    #define EGO_ASSERT_FAIL() ((void)0)
    #define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE) ((void)0)
#endif
