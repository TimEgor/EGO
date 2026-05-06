#pragma once

#include "EgoPlugin/ExternalModuleCore.h"

#include "EgoDemoFramework/DemoController.h"

#define EGO_ENGINE_MODULE()                                                                     \
namespace ego::demo                                                                             \
{                                                                                               \
    static void InitDemoModule(const PluginModuleBindingBridge& _bindings)                      \
    {                                                                                           \
        DemoControllerCore::GetInstance().setController(_bindings.getBinding<DemoController>());\
    }                                                                                           \
                                                                                                \
    EGO_MODULE(InitDemoModule, nullptr)                                                         \
}
