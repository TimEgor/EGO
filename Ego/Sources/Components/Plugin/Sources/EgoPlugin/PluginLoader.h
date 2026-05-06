#pragma once

#include "EgoCore/Reference/Reference.h"

#include "Plugin.h"

namespace ego
{
    class PluginModuleBindingBridge;
    class PhysicsCore;

    class Plugin;
    class FileName;

    class PluginLoader : public STDDestroyMTCountable
    {
    public:
        PluginLoader() = default;

        PluginModulePointer loadModule(PluginModuleID _moduleID, const FileName& _moduleName, PluginModuleBindingBridge& _bindings);
        void unloadModule(void* _moduleHandle, const FileName& _moduleName);

        virtual PluginPointer loadPlugin(const PluginModulePointer& _module, const char* _typeName) = 0;

    protected:
        using ModuleInitFunctionPtr = void(*)(const PluginModuleInfo&, PluginModuleBindingBridge&);
        using ModuleReleaseFunctionPtr = void(*)();

        using PluginCreatingFunctionPtr = Plugin*(*)(const PluginModulePointer&);

        virtual void* loadNativeModule(const FileName& _moduleName) = 0;
        virtual void unloadNativeModule(void* _moduleHandle, const FileName& _moduleName) = 0;

        virtual void initModule(void* _moduleHandle, const PluginModuleInfo& _moduleInfo, PluginModuleBindingBridge& _bindings) = 0;
        virtual void releaseModule(void* _moduleHandle) = 0;
    };

    EGO_REFERENCE(PluginLoader);
}
