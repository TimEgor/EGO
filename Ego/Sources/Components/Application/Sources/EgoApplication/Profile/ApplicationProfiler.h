#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Pointer/Pointer.h"

namespace ego::profile
{
    class Profiler;

    EGO_POINTER(Profiler);
} // namespace ego::profile

namespace ego
{
    class ProfilerPlugin;

    EGO_POINTER(ProfilerPlugin);
} // namespace ego

namespace ego::application
{
    class ApplicationProfiler final
    {
    public:
        ApplicationProfiler() = default;
        ~ApplicationProfiler();

        bool init(const FileName& _pluginModuleName);
        void release();

    private:
        ProfilerPluginPointer m_plugin = nullptr;
        profile::ProfilerPointer m_profiler = nullptr;
    };

    EGO_POINTER(ApplicationProfiler);
} // namespace ego::application
