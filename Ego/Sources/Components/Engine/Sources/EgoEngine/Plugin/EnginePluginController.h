#pragma once

#include "EgoPlugin/PluginController.h"

#include "EnginePlugin.h"

namespace ego::engine
{
	class EnginePluginController final
	{
	public:
		EnginePluginController() = default;

		bool init();

		template <typename TPlugin>
		SharedPointer<TPlugin> loadEnginePlugin(const FileName& _moduleName)
		{
			static_assert(std::is_base_of_v<EnginePlugin, TPlugin>);
			return PluginControllerCore::GetInstance().getPluginController().loadPlugin<TPlugin>(_moduleName);
		}
	};
}
