#include "PluginModule.h"

#include "EgoCore/Assert/AssertCore.h"

#include "PluginController.h"

ego::PluginModule::PluginModule(const PluginModuleInfo& _info)
    : m_info(_info) {}

ego::PluginModule::~PluginModule() = default;

void ego::PluginModuleDeleter::operator()(PluginModule* _pluginModule) const
{
    if (!_pluginModule)
    {
        return;
    }

    PluginController::PluginControllerAccessor::ReleasePluginModule(_pluginModule);
    delete _pluginModule;
}

void ego::PluginModuleCore::init(const PluginModuleBindingBridge& _bindings)
{
    for (const auto& initializer : m_initializers)
    {
        EGO_ASSERT(initializer);
        initializer(_bindings);
    }
}

void ego::PluginModuleCore::release()
{
    for (const auto& releaser : m_releasers)
    {
        EGO_ASSERT(releaser);
        releaser();
    }
}

void ego::PluginModuleCore::addInitializer(PluginModuleInitializerFunction _function)
{
    EGO_ASSERT(_function);
    m_initializers.push_back(_function);
}

void ego::PluginModuleCore::addReleaser(PluginModuleReleaserFunction _function)
{
    EGO_ASSERT(_function);
    m_releasers.push_back(_function);
}

ego::PluginModuleRegistrator::PluginModuleRegistrator(
    PluginModuleInitializerFunction _init,
    PluginModuleReleaserFunction _release
)
{
    if (_init)
    {
        PluginModuleCore::GetInstance().addInitializer(_init);
    }

    if (_release)
    {
        PluginModuleCore::GetInstance().addReleaser(_release);
    }
}

void ego::PluginModuleCore::setInfo(const PluginModuleInfo& _info)
{
    if (m_isInfoSet)
    {
        EGO_ASSERT_FAIL_MESSAGE("Plugin module info has been already set.");
        return;
    }

    m_isInfoSet = true;

    m_info = _info;
}

const ego::PluginModuleInfo& ego::PluginModuleCore::getInfo() const
{
    EGO_ASSERT_MESSAGE(m_isInfoSet, "Plugin module info hasn't been set yet.");
    return m_info;
}
