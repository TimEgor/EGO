#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoEngineFramework/Project.h"

namespace ego
{
    class XmlNode;
}

namespace ego::engine_framework
{
    class ProjectReader final : public NonInstanceable
    {
    public:
        static bool ReadFromFile(const FileName& _fileName, Project& _project);

    private:
        static bool ReadFromRootNode(const XmlNode& _rootNode, Project& _project);
        static bool ReadAssetDirectories(const XmlNode& _rootNode, Project& _project);
        static bool ReadPluginDirectories(const XmlNode& _rootNode);
        static bool ReadPlugins(const XmlNode& _rootNode, Project& _project);
        static bool ReadPluginNode(const XmlNode& _pluginNode, Project& _project);
        static bool ReadEngineLogic(const XmlNode& _rootNode, Project& _project);
        static bool ReadEngineLogicPlugins(const XmlNode& _rootNode, Project& _project);
        static bool ReadEngineLogicPluginNode(const XmlNode& _pluginNode, Project& _project);
        static FileName ResolvePluginModuleName(const std::string& _type, const std::string& _name, const FileName& _moduleName);
        static FileName ResolveEngineLogicPluginModuleName(const std::string& _name, const FileName& _moduleName);
    };
} // namespace ego::engine_framework
