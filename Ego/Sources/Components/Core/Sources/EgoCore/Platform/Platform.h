#pragma once

#include <cstddef>

#include "EgoCore/RTTI/RTTI.h"
#include "FileSystem/FileSystem.h"
#include "Input/InputDeviceController.h"
#include "Window/WindowSystem.h"

namespace ego
{
    using PlatformType = rtti::TypeMetaInfoID;
    constexpr PlatformType InvalidPlatformType = 0;

    class Platform
    {
    public:
        using DynamicLibraryHandle = void*;
        static constexpr DynamicLibraryHandle InvalidDynamicLibraryHandle = nullptr;

        struct OpenFileDialogFilter final
        {
            const char* m_name = nullptr;
            const char* m_pattern = nullptr;
        };

        struct OpenFileDialogParams final
        {
            const char* m_title = nullptr;
            const char* m_defaultExtension = nullptr;
            const OpenFileDialogFilter* m_filters = nullptr;
            std::size_t m_filterCount = 0;
            void* m_ownerWindowHandle = nullptr;
        };

        Platform() = default;
        virtual ~Platform() = default;

        virtual bool init() = 0;
        virtual void release() = 0;

        virtual FileSystemPointer getFileSystem() = 0;
        virtual InputDeviceController& getInputDeviceController() = 0;
        virtual WindowSystem& getWindowSystem() = 0;
        virtual FileName selectOpenFile(const OpenFileDialogParams& _params) const = 0;

        virtual DynamicLibraryHandle loadDynamicLibrary(const FileName& _libraryPath) = 0;
        virtual void unloadDynamicLibrary(DynamicLibraryHandle _libraryHandle, const FileName& _libraryPath) = 0;
        virtual void* getDynamicLibrarySymbol(DynamicLibraryHandle _libraryHandle, const char* _symbolName) = 0;

        EGO_RTTI_VIRTUAL_BASE(Platform);
    };

    EGO_POINTER(Platform);
} // namespace ego
