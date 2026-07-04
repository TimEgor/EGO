#pragma once

#include "EgoCore/Platform/Platform.h"
#include "GlobalContext.h"

namespace ego::context
{
    class PlatformContext final : public GlobalContext
    {
    public:
        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
        };

        PlatformContext() = default;
        ~PlatformContext() override = default;

        bool init(const InitData& _initData);
        void release();

        PlatformPointer getPlatformPointer() const;
        Platform& getPlatform() const;

        EGO_RTTI_VIRTUAL(PlatformContext, GlobalContext);

    private:
        PlatformPointer m_platform = nullptr;
    };

    EGO_POINTER(PlatformContext);

    PlatformContextPointer GetPlatformContextPointer();
    PlatformContext& GetPlatformContext();

    PlatformPointer GetPlatformPointer();
    Platform& GetPlatform();
} // namespace ego::context
