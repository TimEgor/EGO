#pragma once

#include "EgoPlugin/PluginController.h"

#include "DemoPlugin.h"

namespace ego::demo
{
	class DemoPluginController final
	{
	public:
		DemoPluginController() = default;
        ~DemoPluginController() { release(); }

		bool init();
        void release();

		template <typename TPlugin>
		FileName selectDemoPluginModule()
		{
			static_assert(std::is_base_of_v<DemoPlugin, TPlugin>);
			return PluginControllerCore::GetInstance().getPluginController()->selectPluginModule<TPlugin>();
		}

		template <typename TPlugin>
		SharedPointer<TPlugin> loadDemoPlugin(const FileName& _moduleName)
		{
			static_assert(std::is_base_of_v<DemoPlugin, TPlugin>);
			return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>(_moduleName);
		}

		template <typename TPlugin>
		SharedPointer<TPlugin> loadDemoPlugin()
		{
			static_assert(std::is_base_of_v<DemoPlugin, TPlugin>);
			return PluginControllerCore::GetInstance().getPluginController()->loadPlugin<TPlugin>();
		}

    private:
        bool m_isInitialized = false;
	};

	EGO_POINTER(DemoPluginController);
}
