#pragma once

#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/UtilsMacros.h"

#include "PluginModule.h"

#define EGO_MODULE_INIT_FUNC EgoModuleInit
#define EGO_MODULE_RELEASE_FUNC EgoModuleRelease

#define EGO_MODULE(_NAME, _INIT_FUNC, _RELEASE_FUNC) static ego::PluginModuleRegistrator PluginModuleRegestrator_##_NAME(_INIT_FUNC, _RELEASE_FUNC);

#define EGO_MODULE_ENTRY()                                                                                                                                                         \
    extern "C" EGO_DYNAMIC_LIB_EXPORT void __cdecl EGO_MODULE_INIT_FUNC(const ego::PluginModuleInfo& _info, const ego::context::ContextStackInterfacePointer& _contextStack)       \
    {                                                                                                                                                                              \
        EGO_CHECK_RETURN(_contextStack);                                                                                                                                           \
        EGO_CHECK_RETURN(ego::context::ContextStackCore::GetInstance().init(_contextStack));                                                                                       \
                                                                                                                                                                                   \
        ego::PluginModuleState& moduleState = ego::GetPluginModuleState();                                                                                                         \
        moduleState.setInfo(_info);                                                                                                                                                \
        moduleState.init();                                                                                                                                                        \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    extern "C" EGO_DYNAMIC_LIB_EXPORT void __cdecl EGO_MODULE_RELEASE_FUNC()                                                                                                       \
    {                                                                                                                                                                              \
        ego::PluginModuleState& moduleState = ego::GetPluginModuleState();                                                                                                         \
        moduleState.release();                                                                                                                                                     \
        ego::context::ContextStackCore::GetInstance().release();                                                                                                                   \
    }
