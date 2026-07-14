#pragma once

#include <mutex>
#include <type_traits>
#include <unordered_map>

#include "Subsystem.h"
#include "EgoCore/Patterns/Singleton.h"

namespace ego::subsystem
{
    class SubsystemRegistry final
    {
    public:
        SubsystemRegistry() = default;
        ~SubsystemRegistry() = default;

        bool registerSubsystem(const SubsystemPointer& _subsystem);
        void unregisterSubsystem(const SubsystemPointer& _subsystem);
        SubsystemPointer findSubsystem(SubsystemType _subsystemType) const;

    private:
        using SubsystemCollection = std::unordered_map<SubsystemType, SubsystemPointer>;

        mutable std::mutex m_mutex;
        SubsystemCollection m_subsystems;
    };

    EGO_POINTER(SubsystemRegistry);

    class SubsystemLocator final : public Singleton<SubsystemLocator>
    {
    public:
        SubsystemLocator() = default;

        bool bind(const SubsystemRegistryPointer& _registry);
        void unbind();

        SubsystemRegistryPointer getRegistryPointer() const;

        SubsystemPointer findSubsystem(SubsystemType _subsystemType) const;

        template <typename TSubsystem>
        SharedPointer<TSubsystem> findSubsystem() const
        {
            static_assert(std::is_base_of_v<Subsystem, TSubsystem>);

            const SubsystemPointer subsystem = findSubsystem(EGO_SUBSYSTEM_TYPE(TSubsystem));
            return subsystem ? StaticPointerCast<TSubsystem>(subsystem) : nullptr;
        }

    private:
        SubsystemRegistryPointer m_registry = nullptr;
    };

    template <typename TSubsystem>
    SharedPointer<TSubsystem> FindSubsystem()
    {
        return SubsystemLocator::GetInstance().findSubsystem<TSubsystem>();
    }
} // namespace ego::subsystem
