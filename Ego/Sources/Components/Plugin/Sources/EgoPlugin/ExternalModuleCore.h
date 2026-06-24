#pragma once

#include "EgoCore/PlatformMacros.h"
#include "EgoCore/Log/Log.h"
#include "EgoCore/Profile/Profile.h"

#include "PluginController.h"
#include "PluginModule.h"
#include "PluginModuleBindingBridge.h"

#define EGO_MODULE_INIT_FUNC EgoModuleInit
#define EGO_MODULE_RELEASE_FUNC EgoModuleRelease

#define EGO_MODULE(_NAME, _INIT_FUNC, _RELEASE_FUNC) static ego::PluginModuleRegistrator PluginModuleRegestrator_##_NAME(_INIT_FUNC, _RELEASE_FUNC);

#define EGO_CORE_MODULE()                                                                                                                                                          \
    extern "C" EGO_DYNAMIC_LIB_EXPORT void __cdecl EGO_MODULE_INIT_FUNC(const ego::PluginModuleInfo& _info, const ego::PluginModuleBindingBridge& _bindings)                       \
    {                                                                                                                                                                              \
        ego::PluginModuleCore::GetInstance().setInfo(_info);                                                                                                                       \
        ego::PluginModuleCore::GetInstance().init(_bindings);                                                                                                                      \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    extern "C" EGO_DYNAMIC_LIB_EXPORT void __cdecl EGO_MODULE_RELEASE_FUNC()                                                                                                       \
    {                                                                                                                                                                              \
        ego::PluginModuleCore::GetInstance().release();                                                                                                                            \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    namespace ego                                                                                                                                                                  \
    {                                                                                                                                                                              \
        static void InitModuleCore(const PluginModuleBindingBridge& _bindings)                                                                                                     \
        {                                                                                                                                                                          \
            AssertCore::GetInstance().setGenerator(_bindings.getBinding<AssertGenerator>());                                                                                       \
            log::LogCore::GetInstance().setLogger(_bindings.getBinding<log::Logger>());                                                                                            \
            profile::ProfileCore::GetInstance().setController(_bindings.getBinding<profile::ProfilerController>());                                                                \
            PluginControllerCore::GetInstance().init(_bindings.getBinding<PluginController>());                                                                                    \
        }                                                                                                                                                                          \
                                                                                                                                                                                   \
        EGO_MODULE(Core, InitModuleCore, nullptr)                                                                                                                                  \
    }
