#pragma once

#include "EgoCore/Profile/Profile.h"

namespace ego::winpix
{
    class WinPIXProfiler final : public profile::Profiler
    {
    public:
        WinPIXProfiler() = default;

        virtual void beginEvent(const char* _titleName, const char* _contextName = nullptr) override;
        virtual void endEvent() override;
    };

    EGO_POINTER(WinPIXProfiler);
}
