#include "EnginePluginController.h"

#include "EgoPlugin/PluginController.h"

#include "EgoEngine/Engine.h"

bool ego::engine::EnginePluginController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().addBinding(EngineCore::GetInstance().getEngine());
    }

    m_isInitialized = true;

    return true;
}

void ego::engine::EnginePluginController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().removeBinding<Engine>();
    }

    m_isInitialized = false;
}
