#include "PluginModuleBindingBridge.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/String/Format.h"

void ego::PluginModuleBindingBridge::addBinding(void* _instance, BindingTypeID _id)
{
    EGO_ASSERT(_id != InvalidBindingTypeID);

    std::lock_guard locker(m_lock);

    if (m_bindings.contains(_id))
    {
        EGO_ASSERT_FAIL_MESSAGE("Binding has already been added.");
        return;
    }

    m_bindings.insert(std::make_pair(_id, _instance));
}

void* ego::PluginModuleBindingBridge::getBinding(BindingTypeID _id) const
{
    EGO_ASSERT(_id != InvalidBindingTypeID);

    std::lock_guard locker(m_lock);

    const auto bindingIter = m_bindings.find(_id);
    if (bindingIter == m_bindings.end())
    {
        EGO_ASSERT_FAIL_MESSAGE("Binding hasn't been added yet.");
        return nullptr;
    }

    return bindingIter->second;
}
