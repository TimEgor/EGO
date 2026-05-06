#pragma once

#include "Window.h"

namespace ego
{
    class MainWindowProvider
    {
    public:
        MainWindowProvider() = default;
        virtual ~MainWindowProvider() = default;

        virtual bool prepareMainWindow(const char* _title, const WindowSize& _size) = 0;

        virtual bool isWindowPlatformProvided() const = 0;
        virtual WindowPointer getMainWindow() const = 0;

        EGO_RTTI_VIRTUAL_BASE(MainWindowProvider);
    };
}
