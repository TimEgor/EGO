#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"

namespace ego::application
{
    class Application : public NonCopyable
    {
    public:
        ~Application() override;

    protected:
        Application() = default;

        bool init();
        void release();

    private:
        bool initSubsystemRegistry();
        void releaseSubsystemRegistry();

        subsystem::SubsystemRegistryPointer m_subsystemRegistry = nullptr;
    };
} // namespace ego::application
