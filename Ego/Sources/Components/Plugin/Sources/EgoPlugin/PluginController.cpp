#include "PluginController.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/UtilsMacros.h"

#include "Loaders/PluginLoaderCreator.h"

ego::PluginModuleID ego::PluginController::GetModuleID(const FileName& _moduleName)
{
    return _moduleName.hash();
}

void ego::PluginController::PluginControllerAccessor::ReleasePluginModule(PluginModule* _pluginModule)
{
    EGO_ASSERT(_pluginModule);

    PluginControllerCore().GetInstance().getPluginController().unloadModule(_pluginModule);
}

void ego::PluginController::PluginControllerAccessor::ReleasePlugin(Plugin* _plugin)
{
    EGO_ASSERT(_plugin);

    PluginControllerCore().GetInstance().getPluginController().unloadPlugin(_plugin);
}

bool ego::PluginController::init()
{
    std::lock_guard lock(m_mutex);

    m_loader = CreatePluginLoader();
    EGO_CHECK_INITIALIZATION(m_loader);

    m_bindingBridge.addBinding(AssertCore::GetInstance().getGenerator());
    m_bindingBridge.addBinding(&PluginControllerCore::GetInstance().getPluginController());

    return true;
}

void ego::PluginController::release()
{
    std::lock_guard lock(m_mutex);

    EGO_ASSERT(m_modules.empty());
    EGO_ASSERT(m_plugins.empty());

    m_modules = ModuleContainer();
    m_plugins = PluginContainer();

    m_loader.reset();
}

const ego::PluginModuleBindingBridge& ego::PluginController::getBindingBridge() const
{
    return m_bindingBridge;
}

ego::PluginModuleBindingBridge& ego::PluginController::getBindingBridge()
{
    return m_bindingBridge;
}

ego::PluginPointer ego::PluginController::loadPlugin(
    const FileName& _moduleName,
    PluginType _pluginType,
    const char* _pluginTypeName
)
{
    EGO_ASSERT(m_loader);
    EGO_ASSERT(_moduleName);

    std::lock_guard lock(m_mutex);

    const PluginModuleID moduleID = GetModuleID(_moduleName);

    PluginModulePointer module = nullptr;

    auto moduleIter = m_modules.find(moduleID);
    if (moduleIter == m_modules.end())
    {
        module = m_loader->loadModule(moduleID, _moduleName, m_bindingBridge);
        moduleIter = m_modules.emplace(moduleID, module).first;
    }
    else
    {
        module = moduleIter->second.lock();
    }

    EGO_ASSERT(module);

    PluginPointer plugin;

    const PluginID pluginID = CalcPluginID(moduleID, _pluginType);
    auto pluginIter = m_plugins.find(pluginID);
    if (pluginIter == m_plugins.end())
    {
        plugin = m_loader->loadPlugin(module, _pluginTypeName);
        if (!plugin)
        {
            return nullptr;
        }

        pluginIter = m_plugins.emplace(pluginID, plugin).first;

        plugin->onLoaded();
    }
    else
    {
        plugin = pluginIter->second.lock();
    }

    return plugin;
}

void ego::PluginController::unloadModule(PluginModule* _module)
{
    std::lock_guard lock(m_mutex);

    EGO_ASSERT(m_loader);
    EGO_ASSERT(_module);

    const PluginModuleInfo& moduleInfo = _module->getInfo();

    const auto moduleIter = m_modules.find(moduleInfo.m_moduleId);
    if (moduleIter != m_modules.end())
    {
        m_modules.erase(moduleIter);
    }
    else
    {
        EGO_ASSERT_FAIL_MESSAGE("Module isn't stored in controller.");
    }

    m_loader->unloadModule(_module, moduleInfo.m_modulePath);
}

void ego::PluginController::unloadPlugin(Plugin* _plugin)
{
    std::lock_guard lock(m_mutex);

    EGO_ASSERT(m_loader);
    EGO_ASSERT(_plugin);

    const auto pluginIter = m_plugins.find(_plugin->getPluginID());
    if (pluginIter != m_plugins.end())
    {
        m_plugins.erase(pluginIter);
    }
    else
    {
        EGO_ASSERT_FAIL_MESSAGE("Plugin isn't stored in controller.");
    }

    PluginModulePointer module = _plugin->getModule();

    EGO_ASSERT(module);

    _plugin->onUnloaded();
}

bool ego::PluginControllerCore::init(PluginController* _pluginController)
{
    EGO_ASSERT(_pluginController);

    if (m_pluginController)
    {
        EGO_ASSERT_FAIL_MESSAGE("Plugin controller has been already inited.");
        return false;
    }

    m_pluginController = _pluginController;

    return m_pluginController;
}

ego::PluginController& ego::PluginControllerCore::getPluginController()
{
    EGO_ASSERT_MESSAGE(m_pluginController, "Plugin controller hasn't been inited.")

    return *m_pluginController;
}
