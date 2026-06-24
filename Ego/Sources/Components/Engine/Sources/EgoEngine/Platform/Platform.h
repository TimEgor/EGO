#pragma once

#include "EgoCore/RTTI/RTTI.h"

#include "FileSystem/FileSystem.h"
#include "Window/MainWindowProvider.h"
#include "PlatformEventController.h"

namespace ego
{
    using PlatformType = rtti::TypeMetaInfoID;
    constexpr PlatformType InvalidPlatformType = 0;

    class Platform
    {
    public:
        Platform() = default;
        virtual ~Platform() = default;

        virtual bool init() = 0;
        virtual void release() = 0;

        virtual MainWindowProvider& getMainWindowProvider() = 0;
        virtual const MainWindowProvider& getMainWindowProvider() const = 0;
        virtual WindowPointer createWindow(const char* _title, const WindowSize& _size) = 0;

        virtual const PlatformEventController& getPlatformEventController() const = 0;
        virtual PlatformEventController& getPlatformEventController() = 0;

        virtual FileSystemPointer getFileSystem() = 0;

        EGO_RTTI_VIRTUAL_BASE(Platform);
    };

    EGO_POINTER(Platform);
} // namespace ego
