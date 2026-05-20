#include "DemoPluginController.h"

#include "EgoPlugin/PluginController.h"

#include "EgoDemoFramework/DemoController.h"

bool ego::demo::DemoPluginController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().addBinding(DemoControllerCore::GetInstance().getController());
    }

    m_isInitialized = true;

    return true;
}

void ego::demo::DemoPluginController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().removeBinding<DemoController>();
    }

    m_isInitialized = false;
}
