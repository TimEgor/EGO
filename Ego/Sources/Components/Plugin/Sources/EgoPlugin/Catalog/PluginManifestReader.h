#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "PluginCatalog.h"

namespace ego
{
    class XmlNode;
} // namespace ego

namespace ego
{
    class PluginManifestReader final : public NonInstanceable
    {
    public:
        struct Context final
        {
            std::string m_platform;
            std::string m_configuration;
        };

        enum class Status
        {
            Loaded,
            NotApplicable,
            InvalidManifest
        };

        struct Result final
        {
            Status m_status = Status::InvalidManifest;
            PluginCatalog::ModuleDesc m_module;
            std::string m_message;

            bool isLoaded() const;
            bool isSkipped() const;
        };

        static Result Read(const std::string& _content, const FileName& _moduleName, const Context& _context);

    private:
        static Result ReadNode(const XmlNode& _manifestNode, const FileName& _moduleName, const Context& _context);
        static bool EqualsNoCase(const std::string& _left, const std::string& _right);
        static bool ContainsPlugin(const PluginCatalog::ModuleDesc& _module, PluginType _pluginType, const std::string& _pluginName);
    };
} // namespace ego
