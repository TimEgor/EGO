#pragma once

#include "EgoPlugin/ExternalModuleCore.h"

#include "EgoEngine/Engine.h"

#define EGO_ENGINE_MODULE()                                                                                                                                                        \
    namespace ego::engine                                                                                                                                                          \
    {                                                                                                                                                                              \
        static void InitEngineModule(const PluginModuleBindingBridge& _bindings)                                                                                                   \
        {                                                                                                                                                                          \
            EngineCore::GetInstance().init(_bindings.getBinding<Engine>());                                                                                                        \
        }                                                                                                                                                                          \
                                                                                                                                                                                   \
        EGO_MODULE(Engine, InitEngineModule, nullptr)                                                                                                                              \
    }
