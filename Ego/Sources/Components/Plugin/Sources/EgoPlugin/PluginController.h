#pragma once

#include <mutex>
#include <unordered_map>

#include "EgoCore/Patterns/NonInstanceable.h"

#include "Plugin.h"
#include "PluginLoader.h"
#include "PluginModuleBindingBridge.h"

namespace ego
{
    class PluginLoader;
    class FileName;

    class PluginController final
    {
        using ModuleContainer = std::unordered_map<PluginModuleID, PluginModuleWeakPointer>;
        using PluginContainer = std::unordered_map<PluginID, PluginWeakPointer>;

    public:
        class PluginControllerAccessor final : public NonInstanceable
        {
            friend PluginModule;
            friend Plugin;

        public:
            static void ReleasePluginModule(PluginModule* _pluginModule);
            static void ReleasePlugin(Plugin* _plugin);
        };

        PluginController() = default;
        ~PluginController() { release(); }

        bool init();
        void release();

        template <typename TPlugin = Plugin>
        SharedPointer<TPlugin> loadPlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            PluginPointer plugin = loadPlugin(_moduleName, TPlugin::GetPluginType(), TPlugin::GetPluginTypeName());

            EGO_ASSERT((rtti::IsObjectBasedOn<TPlugin>(*plugin)));

            return StaticPointerCast<TPlugin>(plugin);
        }

        const PluginModuleBindingBridge& getBindingBridge() const;
        PluginModuleBindingBridge& getBindingBridge();

    protected:
        void unloadModule(PluginModule* _module);
        void unloadPlugin(Plugin* _plugin);

    private:
        PluginPointer loadPlugin(const FileName& _moduleName, PluginType _pluginType, const char* _pluginTypeName);

        static PluginModuleID GetModuleID(const FileName& _moduleName);

        mutable std::recursive_mutex m_mutex;

        PluginModuleBindingBridge m_bindingBridge;

        ModuleContainer m_modules;
        PluginContainer m_plugins;

        PluginLoaderReference m_loader;
    };

    class PluginControllerCore final : public Singleton<PluginControllerCore>
    {
    public:
        PluginControllerCore() = default;

        bool init(PluginController* _pluginController);
        PluginController& getPluginController();

    private:
        PluginController* m_pluginController = nullptr;
    };
}
