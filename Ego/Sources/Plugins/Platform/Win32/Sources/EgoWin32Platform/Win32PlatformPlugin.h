#pragma once

#include "EgoEngine/Platform/PlatformPlugin.h"

namespace ego::win32
{
    class Win32PlatformPlugin final : public PlatformPlugin
    {
    public:
        Win32PlatformPlugin(const PluginModulePointer& _module);

        virtual Platform* createPlatform(void* _platformNativeInstance) override;

        EGO_PLUGIN(Win32PlatformPlugin, PlatformPlugin);
    };

    EGO_POINTER(Win32PlatformPlugin);
}
