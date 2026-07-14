#pragma once

#include "EgoCore/UtilsMacros.h"

#ifndef EGO_ENABLE_PROFILING
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE) || defined(EGO_CONFIG_PROFILE)
        #define EGO_ENABLE_PROFILING 1
    #else
        #define EGO_ENABLE_PROFILING 0
    #endif
#endif

namespace ego::profile
{
    void BeginEvent(const char* _titleName, const char* _contextName = nullptr);
    void EndEvent();

    class ProfileEvent final
    {
    public:
        ProfileEvent(const char* _titleName, const char* _contextName = nullptr);
        ~ProfileEvent();

    private:
        bool m_isActive = false;
    };
} // namespace ego::profile

#if EGO_ENABLE_PROFILING
    #define EGO_PROFILE_SCOPE_NAME(SCOPE) EGO_CONCAT_DEF(profileEvent_, SCOPE)
    #define EGO_PROFILE_GENERIC_SCOPE_NAME() EGO_PROFILE_SCOPE_NAME(EGO_COUNTER)

    #define EGO_PROFILE_BEGIN_EVENT(TITLE) ego::profile::BeginEvent(TITLE)
    #define EGO_PROFILE_BEGIN_EVENT_CONTEXT(TITLE, CONTEXT) ego::profile::BeginEvent(TITLE, CONTEXT)
    #define EGO_PROFILE_END_EVENT() ego::profile::EndEvent()

    #define EGO_PROFILE_BLOCK_EVENT(TITLE) ego::profile::ProfileEvent EGO_PROFILE_GENERIC_SCOPE_NAME()(TITLE)
    #define EGO_PROFILE_BLOCK_EVENT_CONTEXT(TITLE, CONTEXT) ego::profile::ProfileEvent EGO_PROFILE_GENERIC_SCOPE_NAME()(TITLE, CONTEXT)
    #define EGO_PROFILE_FUNCTION() EGO_PROFILE_BLOCK_EVENT(EGO_FUNCTION_NAME)
#else
    #define EGO_PROFILE_SCOPE_NAME(SCOPE)
    #define EGO_PROFILE_GENERIC_SCOPE_NAME()

    #define EGO_PROFILE_BEGIN_EVENT(TITLE) ((void)0)
    #define EGO_PROFILE_BEGIN_EVENT_CONTEXT(TITLE, CONTEXT) ((void)0)
    #define EGO_PROFILE_END_EVENT() ((void)0)

    #define EGO_PROFILE_BLOCK_EVENT(TITLE) ((void)0)
    #define EGO_PROFILE_BLOCK_EVENT_CONTEXT(TITLE, CONTEXT) ((void)0)
    #define EGO_PROFILE_FUNCTION() ((void)0)
#endif
