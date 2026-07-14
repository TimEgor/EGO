#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoPlugin/Plugin.h"

namespace ego
{
    class PluginCatalog final : public NonCopyable
    {
    public:
        using RegistrationID = uint64_t;

        inline static constexpr RegistrationID InvalidRegistrationID = 0;

        struct PluginDesc final
        {
            PluginType m_type = rtti::InvalidTypeMetaInfoID;
            std::string m_name;
        };

        struct ModuleDesc final
        {
            FileName m_moduleName;
            std::vector<PluginDesc> m_plugins;
        };

        using ModuleCollection = std::vector<ModuleDesc>;

        PluginCatalog();
        ~PluginCatalog() override;

        RegistrationID registerModules(const ModuleCollection& _modules);
        bool unregisterModules(RegistrationID _registrationID);
        void clear();

        FileName resolve(PluginType _pluginType) const;
        FileName resolve(PluginType _pluginType, std::string_view _pluginName) const;

    private:
        class Storage;

        std::unique_ptr<Storage> m_storage;
    };
} // namespace ego
