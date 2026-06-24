#pragma once

#include "EgoPlugin/PluginController.h"

#include "EnginePlugin.h"

namespace ego::engine
{
    class EnginePluginController final
    {
    public:
        EnginePluginController() = default;
        ~EnginePluginController()
        {
            release();
        }

        bool init();
        void release();

        template <typename TPlugin>
        FileName selectEnginePluginModule()
        {
            static_assert(std::is_base_of_v<EnginePlugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->selectPluginModule<TPlugin>();
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadEnginePlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<EnginePlugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>(_moduleName);
        }

    private:
        bool m_isInitialized = false;
    };

    EGO_POINTER(EnginePluginController);
} // namespace ego::engine
