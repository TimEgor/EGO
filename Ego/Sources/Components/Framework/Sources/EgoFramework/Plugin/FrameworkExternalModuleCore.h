#pragma once

#include "EgoPlugin/ExternalModuleCore.h"

#include "EgoFramework/Framework.h"

#define EGO_FRAMEWORK_MODULE()                                                                                                                                                     \
    namespace ego::framework                                                                                                                                                       \
    {                                                                                                                                                                              \
        static void InitFrameworkModule(const PluginModuleBindingBridge& _bindings)                                                                                                \
        {                                                                                                                                                                          \
            FrameworkCore::GetInstance().init(_bindings.getBinding<Framework>());                                                                                                  \
        }                                                                                                                                                                          \
                                                                                                                                                                                   \
        EGO_MODULE(Framework, InitFrameworkModule, nullptr)                                                                                                                        \
    }
