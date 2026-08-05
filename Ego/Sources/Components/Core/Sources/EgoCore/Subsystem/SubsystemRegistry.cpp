#include "SubsystemRegistry.h"

#include "EgoCore/Assert/Assert.h"

bool ego::subsystem::SubsystemRegistry::registerSubsystem(const SubsystemPointer& _subsystem)
{
    EGO_ASSERT(_subsystem);
    if (!_subsystem)
    {
        return false;
    }

    const SubsystemType subsystemType = _subsystem->getType();
    EGO_ASSERT(subsystemType != InvalidSubsystemType);
    if (subsystemType == InvalidSubsystemType)
    {
        return false;
    }

    std::lock_guard lock(m_mutex);

    const auto subsystemIt = m_subsystems.find(subsystemType);
    if (subsystemIt == m_subsystems.end())
    {
        m_subsystems.emplace(subsystemType, _subsystem);
        return true;
    }

    if (subsystemIt->second.get() == _subsystem.get())
    {
        return true;
    }

    EGO_ASSERT_FAIL_MESSAGE("Subsystem has been already registered.");
    return false;
}

void ego::subsystem::SubsystemRegistry::unregisterSubsystem(const SubsystemPointer& _subsystem)
{
    EGO_ASSERT(_subsystem);
    if (!_subsystem)
    {
        return;
    }

    const SubsystemType subsystemType = _subsystem->getType();
    EGO_ASSERT(subsystemType != InvalidSubsystemType);
    if (subsystemType == InvalidSubsystemType)
    {
        return;
    }

    std::lock_guard lock(m_mutex);

    const auto subsystemIt = m_subsystems.find(subsystemType);
    if (subsystemIt == m_subsystems.end())
    {
        EGO_ASSERT_FAIL_MESSAGE("Subsystem isn't registered.");
        return;
    }

    if (subsystemIt->second.get() != _subsystem.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Subsystem unregister request doesn't match registered subsystem.");
        return;
    }

    Subsystem::SubsystemAccessor::NotifyUnregistered(*_subsystem);

    m_subsystems.erase(subsystemType);
}

ego::subsystem::SubsystemPointer ego::subsystem::SubsystemRegistry::findSubsystem(SubsystemType _subsystemType) const
{
    if (_subsystemType == InvalidSubsystemType)
    {
        return nullptr;
    }

    std::lock_guard lock(m_mutex);

    const auto subsystemIt = m_subsystems.find(_subsystemType);
    return subsystemIt != m_subsystems.end() ? subsystemIt->second : nullptr;
}

bool ego::subsystem::SubsystemLocator::bind(const SubsystemRegistryPointer& _registry)
{
    EGO_ASSERT(_registry);
    if (!_registry)
    {
        return false;
    }

    if (m_registry && m_registry.get() != _registry.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Subsystem locator has been already bound.");
        return false;
    }

    m_registry = _registry;
    return true;
}

void ego::subsystem::SubsystemLocator::unbind()
{
    m_registry = nullptr;
}

ego::subsystem::SubsystemRegistryPointer ego::subsystem::SubsystemLocator::getRegistryPointer() const
{
    return m_registry;
}

ego::subsystem::SubsystemPointer ego::subsystem::SubsystemLocator::findSubsystem(SubsystemType _subsystemType) const
{
    if (_subsystemType == InvalidSubsystemType)
    {
        return nullptr;
    }

    return m_registry ? m_registry->findSubsystem(_subsystemType) : nullptr;
}
