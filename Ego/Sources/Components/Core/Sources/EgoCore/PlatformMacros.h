#pragma once

#include "UtilsMacros.h"

#if defined(WIN32) || defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#define EGO_WIN_PLATFORM

#define EGO_DYNAMIC_LIB_EXPORT __declspec(dllexport)
#define EGO_DYNMIC_LIB_IMPORT __declspec(dllimport)
#define EGO_DYNAMIC_LIB_EXT dll

#if defined(WIN64) || defined(_WIN64)
#define EGO_PLATFORM x64
#else
#define EGO_PLATFORM Win32
#endif

#else
#define EGO_DYNAMIC_LIB_EXPORT
#define EGO_DYNMIC_LIB_IMPORT

#define EGO_DYNAMIC_LIB_EXT

#define EGO_PLATFORM
#endif

#define EGO_DYNAMIC_LIB_EXT_NAME EGO_TO_STRING_DEF(EGO_DYNAMIC_LIB_EXT)
#define EGO_DYNAMIC_LIB_FULL_EXT_NAME EGO_TO_STRING_DEF(.EGO_DYNAMIC_LIB_EXT)

#define EGO_PLATFORM_NAME EGO_TO_STRING_DEF(EGO_PLATFORM)
