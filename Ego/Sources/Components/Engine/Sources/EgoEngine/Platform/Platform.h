#pragma once

#include "EgoCore/RTTI/RTTI.h"

#include "FileSystem/FileSystem.h"

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

        virtual FileSystemPointer getFileSystem() = 0;

        EGO_RTTI_VIRTUAL_BASE(Platform);
    };

    EGO_POINTER(Platform);
} // namespace ego
