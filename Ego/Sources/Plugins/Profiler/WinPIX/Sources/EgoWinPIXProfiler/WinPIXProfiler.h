#pragma once

#include "EgoCore/Profile/ProfileController.h"

namespace ego::winpix
{
    class WinPIXProfiler final : public profile::Profiler
    {
    public:
        WinPIXProfiler() = default;

        void beginEvent(const char* _titleName, const char* _contextName = nullptr) override;
        void endEvent() override;
    };

    EGO_POINTER(WinPIXProfiler);
} // namespace ego::winpix
