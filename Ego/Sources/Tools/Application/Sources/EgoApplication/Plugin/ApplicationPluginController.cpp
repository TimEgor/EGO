#include "ApplicationPluginController.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"

#include "EgoApplication/Application.h"

bool ego::application::ApplicationPluginController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (!pluginController)
    {
        m_pluginController = new PluginController();
        if (!m_pluginController || !PluginControllerCore::GetInstance().init(m_pluginController))
        {
            return false;
        }

        m_isPluginControllerCoreInitialized = true;
        if (!m_pluginController->init())
        {
            PluginControllerCore::GetInstance().release();
            m_pluginController = nullptr;
            m_isPluginControllerCoreInitialized = false;
            return false;
        }

        pluginController = m_pluginController;
    }

    pluginController->getBindingBridge().addBinding(ApplicationCore::GetInstance().getApplication());
    m_isInitialized = true;

    return true;
}

void ego::application::ApplicationPluginController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (pluginController)
    {
        pluginController->getBindingBridge().removeBinding<Application>();
    }

    if (m_isPluginControllerCoreInitialized)
    {
        PluginControllerCore::GetInstance().release();
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
        m_isPluginControllerCoreInitialized = false;
    }

    m_pluginController = nullptr;
    m_isInitialized = false;
}
