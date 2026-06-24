#pragma once

#if defined(WIN32) || defined(_WIN32)

    #include "EgoPlugin/PluginLoader.h"

namespace ego
{
    class Win32PluginLoader final : public PluginLoader
    {
    public:
        Win32PluginLoader() = default;

        FileName selectPluginModule(const char* _typeName) override;
        PluginPointer loadPlugin(const PluginModulePointer& _module, const char* _typeName) override;

    protected:
        void* loadNativeModule(const FileName& _moduleName) override;
        void unloadNativeModule(void* _moduleHandle, const FileName& _moduleName) override;

        void initModule(void* _moduleHandle, const PluginModuleInfo& _info, PluginModuleBindingBridge& _bindings) override;
        void releaseModule(void* _moduleHandle) override;
    };
} // namespace ego

#endif
