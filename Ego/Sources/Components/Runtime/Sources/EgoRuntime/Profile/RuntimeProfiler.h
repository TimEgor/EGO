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

namespace ego::runtime
{
    class RuntimeProfiler final
    {
    public:
        RuntimeProfiler() = default;
        ~RuntimeProfiler();

        bool init(const FileName& _pluginModuleName);

    private:
        void release();

        ProfilerPluginPointer m_plugin = nullptr;
        profile::ProfilerPointer m_profiler = nullptr;
    };

    EGO_POINTER(RuntimeProfiler);
} // namespace ego::runtime
