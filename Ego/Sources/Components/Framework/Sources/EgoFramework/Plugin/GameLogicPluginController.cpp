#include "GameLogicPluginController.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"

#include "EgoFramework/Framework.h"

bool ego::framework::GameLogicPluginController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        const FrameworkPointer framework = FrameworkCore::GetInstance().getFramework();
        EGO_CHECK_RETURN_FALSE(framework);

        pluginController->getBindingBridge().addBinding(framework);
    }

    m_isInitialized = true;

    return true;
}

void ego::framework::GameLogicPluginController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().removeBinding<Framework>();
    }

    m_isInitialized = false;
}
