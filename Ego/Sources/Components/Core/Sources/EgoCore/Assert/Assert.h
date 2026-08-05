#pragma once

#include <cassert>

#include "AssertController.h"
#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#ifndef EGO_ENABLE_ASSERTS
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE)
        #define EGO_ENABLE_ASSERTS 1
    #else
        #define EGO_ENABLE_ASSERTS 0
    #endif
#endif

#if EGO_ENABLE_ASSERTS
    #define EGO_ASSERT(_CONDITION)                                                                                                                             \
        if (!(_CONDITION))                                                                                                                                     \
        {                                                                                                                                                      \
            ego::AssertControllerPointer controller = ego::GetAssertController();                                                                              \
            if (!controller || !controller->generateError(EGO_TO_STRING(_CONDITION), EGO_FILE, EGO_LINE))                                                      \
            {                                                                                                                                                  \
                assert(_CONDITION);                                                                                                                            \
            }                                                                                                                                                  \
        }

    #define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE)                                                                                                           \
        if (!(_CONDITION))                                                                                                                                     \
        {                                                                                                                                                      \
            ego::AssertControllerPointer controller = ego::GetAssertController();                                                                              \
            if (!controller || !controller->generateError(_MESSAGE, EGO_FILE, EGO_LINE))                                                                       \
            {                                                                                                                                                  \
                assert((_CONDITION) && (_MESSAGE));                                                                                                            \
            }                                                                                                                                                  \
        }

    #define EGO_ASSERT_FAIL()                                                                                                                                  \
        ego::AssertControllerPointer controller = ego::GetAssertController();                                                                                  \
        if (!controller || !controller->generateError("FAIL", EGO_FILE, EGO_LINE))                                                                             \
        {                                                                                                                                                      \
            assert(false);                                                                                                                                     \
        }

    #define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE)                                                                                                                  \
        ego::AssertControllerPointer controller = ego::GetAssertController();                                                                                  \
        if (!controller || !controller->generateError(_MESSAGE, EGO_FILE, EGO_LINE))                                                                           \
        {                                                                                                                                                      \
            assert(false && (_MESSAGE));                                                                                                                       \
        }

#else
    #define EGO_ASSERT(_CONDITION) ((void)0)
    #define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE) ((void)0)
    #define EGO_ASSERT_FAIL() ((void)0)
    #define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE) ((void)0)
#endif

#define EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(_CHECK_VAL, _MESSAGE)                                                                                          \
    EGO_CHECK_RETURN_CALL_VALUE(_CHECK_VAL, EGO_ASSERT_FAIL_MESSAGE(_MESSAGE); release(), false)
#define EGO_CHECK_INITIALIZATION_ASSERT(_CHECK_VAL) EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(_CHECK_VAL, EGO_TO_STRING(_CHECK_VAL))
