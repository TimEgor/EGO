#pragma once

#include <mutex>
#include <unordered_map>

#include "EgoCore/Assert/AssertCore.h"
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
            friend struct PluginModuleDeleter;
            friend struct PluginDeleter;

            static void ReleasePluginModule(PluginController& _controller, PluginModule* _pluginModule);
            static void ReleasePlugin(PluginController& _controller, Plugin* _plugin);
        };

        PluginController() = default;
        ~PluginController() { release(); }

        bool init();
        void release();

        FileName selectPluginModule(const char* _pluginTypeName);

        template <typename TPlugin>
        FileName selectPluginModule()
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            return selectPluginModule(TPlugin::GetPluginTypeName());
        }

        template <typename TPlugin = Plugin>
        SharedPointer<TPlugin> loadPlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            PluginPointer plugin = loadPlugin(_moduleName, TPlugin::GetPluginType(), TPlugin::GetPluginTypeName());
            EGO_CHECK_RETURN_NULL(plugin);

            EGO_ASSERT((rtti::IsObjectBasedOn<TPlugin>(*plugin)));

            return StaticPointerCast<TPlugin>(plugin);
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadPlugin()
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);

            const FileName moduleName = selectPluginModule<TPlugin>();
            EGO_CHECK_RETURN_NULL(moduleName);

            return loadPlugin<TPlugin>(moduleName);
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

    EGO_POINTER(PluginController);
    EGO_WEAK_POINTER(PluginController);

    class PluginControllerCore final : public Singleton<PluginControllerCore>
    {
    public:
        PluginControllerCore() = default;

        bool init(const PluginControllerPointer& _pluginController);
        void release();
        PluginControllerPointer getPluginController() const;

    private:
        PluginControllerPointer m_pluginController = nullptr;
    };
}
