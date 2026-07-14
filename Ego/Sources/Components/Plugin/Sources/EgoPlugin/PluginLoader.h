#pragma once

#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Reference/Reference.h"

#include "Plugin.h"

namespace ego
{
    class Platform;

    class Plugin;
    class FileName;
    EGO_POINTER(Platform);

    class PluginLoader : public STDDestroyMTCountable
    {
    public:
        PluginLoader() = default;

        FileName selectPluginModule(const char* _typeName);

        PluginModulePointer loadModule(PluginModuleID _moduleID, const FileName& _moduleName, const PluginControllerWeakPointer& _pluginController);
        void unloadModule(void* _moduleHandle, const FileName& _moduleName);

        PluginPointer loadPlugin(const PluginModulePointer& _module, const char* _typeName);

    private:
        using ModuleInitFunctionPtr = void (*)(const PluginModuleInfo&, const subsystem::SubsystemRegistryPointer&);
        using ModuleReleaseFunctionPtr = void (*)();

        using PluginCreatingFunctionPtr = Plugin* (*)(const PluginModulePointer&);

        void initModule(void* _moduleHandle, const PluginModuleInfo& _moduleInfo);
        void releaseModule(void* _moduleHandle);
    };

    EGO_REFERENCE(PluginLoader);
} // namespace ego
