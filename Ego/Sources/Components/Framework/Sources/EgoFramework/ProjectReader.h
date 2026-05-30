#pragma once

#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoCore/FileName/FileName.h"
#include "EgoFramework/Project.h"

namespace ego
{
    class XmlNode;
}

namespace ego::framework
{
    class ProjectReader final : public NonInstanceable
    {
    public:
        static bool ReadFromFile(const FileName& _fileName, Project& _project);

    private:
        static bool ReadFromRootNode(const XmlNode& _rootNode, Project& _project);
        static void ReadAssetDirectories(const XmlNode& _rootNode, Project& _project);
        static void ReadPluginDirectories(const XmlNode& _rootNode, Project& _project);
        static void ReadPlugins(const XmlNode& _rootNode, Project& _project);
        static void ReadGameLogic(const XmlNode& _rootNode, Project& _project);
        static void ReadGameLogicPlugins(const XmlNode& _rootNode, Project& _project);
    };
}
