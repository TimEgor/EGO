#pragma once

#include "EgoPlugin/ExternalModuleCore.h"

#include "EgoApplication/Application.h"

#define EGO_APPLICATION_MODULE()                                                                                                                                                   \
    namespace ego::application                                                                                                                                                     \
    {                                                                                                                                                                              \
        static void InitApplicationModule(const PluginModuleBindingBridge& _bindings)                                                                                              \
        {                                                                                                                                                                          \
            ApplicationCore::GetInstance().init(_bindings.getBinding<Application>());                                                                                              \
        }                                                                                                                                                                          \
                                                                                                                                                                                   \
        EGO_MODULE(Application, InitApplicationModule, nullptr)                                                                                                                    \
    }
