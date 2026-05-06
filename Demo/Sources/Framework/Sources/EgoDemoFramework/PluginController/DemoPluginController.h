#pragma once

#include "EgoPlugin/PluginController.h"

#include "DemoPlugin.h"

namespace ego::demo
{
	class DemoPluginController final
	{
	public:
		DemoPluginController() = default;

		bool init();

		template <typename TPlugin>
		SharedPointer<TPlugin> loadEnginePlugin(const FileName& _moduleName)
		{
			static_assert(std::is_base_of_v<DemoPlugin, TPlugin>);
			return PluginControllerCore::GetInstance().getPluginController().loadPlugin<TPlugin>(_moduleName);
		}
	};
}
