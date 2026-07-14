#pragma once

#include <mutex>
#include <shared_mutex>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::profile
{
    class Profiler
    {
    public:
        Profiler() = default;
        virtual ~Profiler() = default;

        virtual void beginEvent(const char* _titleName, const char* _contextName = nullptr) = 0;
        virtual void endEvent() = 0;
    };

    EGO_POINTER(Profiler);

    class ProfilerController final : public NonCopyable
    {
    public:
        ProfilerController() = default;

        bool setProfiler(const ProfilerPointer& _profiler);
        void resetProfiler(const ProfilerPointer& _profiler = nullptr);

        ProfilerPointer getProfiler() const;

        void beginEvent(const char* _titleName, const char* _contextName = nullptr) const;
        void endEvent() const;

    private:
        mutable std::shared_mutex m_lock;
        ProfilerPointer m_profiler = nullptr;
    };

    EGO_POINTER(ProfilerController);
} // namespace ego::profile
