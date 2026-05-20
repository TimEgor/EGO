#pragma once

#include "EgoPlugin/ExternalModuleCore.h"

#include "EgoDemoFramework/DemoController.h"

#define EGO_DEMO_MODULE()                                                                       \
namespace ego::demo                                                                             \
{                                                                                               \
    static void InitDemoModule(const PluginModuleBindingBridge& _bindings)                      \
    {                                                                                           \
        DemoControllerCore::GetInstance().init(_bindings.getBinding<DemoController>());         \
    }                                                                                           \
                                                                                                \
    EGO_MODULE(Demo, InitDemoModule, nullptr)                                                   \
}
