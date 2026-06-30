#pragma once

#include "EgoPlugin/PluginController.h"

namespace ego::application
{
    class ApplicationPluginController final
    {
    public:
        ApplicationPluginController() = default;
        ~ApplicationPluginController()
        {
            release();
        }

        bool init();
        void release();

        template <typename TPlugin>
        FileName selectPluginModule()
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->selectPluginModule<TPlugin>();
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadPlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>(_moduleName);
        }

    private:
        PluginControllerPointer m_pluginController = nullptr;
        bool m_isPluginControllerCoreInitialized = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ApplicationPluginController);
} // namespace ego::application
