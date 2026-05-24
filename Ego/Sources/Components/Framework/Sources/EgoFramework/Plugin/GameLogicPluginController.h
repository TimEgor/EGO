#pragma once

#include "EgoPlugin/PluginController.h"

#include "GameLogicPlugin.h"

namespace ego::framework
{
    class GameLogicPluginController final
    {
    public:
        GameLogicPluginController() = default;
        ~GameLogicPluginController() { release(); }

        bool init();
        void release();

        template <typename TPlugin>
        FileName selectGameLogicPluginModule()
        {
            static_assert(std::is_base_of_v<GameLogicPlugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->selectPluginModule<TPlugin>();
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadGameLogicPlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<GameLogicPlugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>(_moduleName);
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadGameLogicPlugin()
        {
            static_assert(std::is_base_of_v<GameLogicPlugin, TPlugin>);
            return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>();
        }

    private:
        bool m_isInitialized = false;
    };

    EGO_POINTER(GameLogicPluginController);
}

