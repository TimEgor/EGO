#include "PluginModule.h"

#include "EgoCore/Assert/AssertCore.h"

#include "PluginController.h"

namespace
{
    ego::PluginModuleState& GetModuleStateStorage()
    {
        static ego::PluginModuleState State;
        return State;
    }
} // namespace

ego::PluginModule::PluginModule(const PluginModuleInfo& _info, const PluginControllerWeakPointer& _pluginController)
    : m_pluginController(_pluginController),
      m_info(_info)
{
}

ego::PluginModule::~PluginModule() = default;

ego::PluginControllerPointer ego::PluginModule::getPluginController() const
{
    return m_pluginController.lock();
}

void ego::PluginModuleDeleter::operator()(PluginModule* _pluginModule) const
{
    if (!_pluginModule)
    {
        return;
    }

    PluginControllerPointer pluginController = _pluginModule->getPluginController();
    EGO_ASSERT(pluginController);

    if (pluginController)
    {
        PluginController::PluginControllerAccessor::ReleasePluginModule(*pluginController, _pluginModule);
    }

    delete _pluginModule;
}

void ego::PluginModuleState::init()
{
    for (const auto& initializer : m_initializers)
    {
        EGO_ASSERT(initializer);
        initializer();
    }
}

void ego::PluginModuleState::release()
{
    for (auto releaserIter = m_releasers.rbegin(); releaserIter != m_releasers.rend(); ++releaserIter)
    {
        const PluginModuleReleaserFunction releaser = *releaserIter;
        EGO_ASSERT(releaser);
        releaser();
    }
}

void ego::PluginModuleState::addInitializer(PluginModuleInitializerFunction _function)
{
    EGO_ASSERT(_function);
    m_initializers.push_back(_function);
}

void ego::PluginModuleState::addReleaser(PluginModuleReleaserFunction _function)
{
    EGO_ASSERT(_function);
    m_releasers.push_back(_function);
}

ego::PluginModuleRegistrator::PluginModuleRegistrator(PluginModuleInitializerFunction _init, PluginModuleReleaserFunction _release)
{
    if (_init)
    {
        GetPluginModuleState().addInitializer(_init);
    }

    if (_release)
    {
        GetPluginModuleState().addReleaser(_release);
    }
}

void ego::PluginModuleState::setInfo(const PluginModuleInfo& _info)
{
    if (m_isInfoSet)
    {
        EGO_ASSERT_FAIL_MESSAGE("Plugin module info has been already set.");
        return;
    }

    m_isInfoSet = true;

    m_info = _info;
}

const ego::PluginModuleInfo& ego::PluginModuleState::getInfo() const
{
    EGO_ASSERT_MESSAGE(m_isInfoSet, "Plugin module info hasn't been set yet.");
    return m_info;
}

ego::PluginModuleState& ego::GetPluginModuleState()
{
    return GetModuleStateStorage();
}
