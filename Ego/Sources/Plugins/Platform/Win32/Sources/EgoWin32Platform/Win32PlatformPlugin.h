#pragma once

#include "EgoEngine/Platform/PlatformPlugin.h"

namespace ego::win32
{
    class Win32PlatformPlugin final : public PlatformPlugin
    {
    public:
        Win32PlatformPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        PlatformPointer createPlatform(void* _platformNativeInstance) override;

        EGO_PLUGIN(Win32PlatformPlugin, PlatformPlugin);
    };

    EGO_POINTER(Win32PlatformPlugin);
} // namespace ego::win32
