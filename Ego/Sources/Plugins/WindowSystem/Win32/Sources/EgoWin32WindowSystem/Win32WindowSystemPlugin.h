#pragma once

#include "EgoApplication/Window/WindowSystemPlugin.h"

namespace ego::win32
{
    class Win32WindowSystemPlugin final : public WindowSystemPlugin
    {
    public:
        Win32WindowSystemPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        WindowSystemPointer createWindowSystem(void* _nativeInstanceHandle) override;

        EGO_PLUGIN(Win32WindowSystemPlugin, WindowSystemPlugin);
    };

    EGO_POINTER(Win32WindowSystemPlugin);
} // namespace ego::win32
