#pragma once

#if defined(WIN32) || defined(_WIN32)

#include "EgoPlugin/PluginLoader.h"

namespace ego
{
	class Win32PluginLoader final : public PluginLoader
	{
	public:
		Win32PluginLoader() = default;

		virtual FileName selectPluginModule(const char* _typeName) override;
		virtual PluginPointer loadPlugin(const PluginModulePointer& _module, const char* _typeName) override;

	protected:
		virtual void* loadNativeModule(const FileName& _moduleName) override;
		virtual void unloadNativeModule(void* _moduleHandle, const FileName& _moduleName) override;

		virtual void initModule(void* _moduleHandle, const PluginModuleInfo& _info, PluginModuleBindingBridge& _bindings) override;
		virtual void releaseModule(void* _moduleHandle) override;
	};
}

#endif
