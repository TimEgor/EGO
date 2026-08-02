#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoEngine/Project/Project.h"

namespace pugi
{
    class xml_node;
}

namespace ego::engine
{
    class ProjectWriter final
        : public NonInstanceable
    {
    public:
        static bool WriteToFile(const FileName& _fileName, const Project& _project);

    private:
        static bool WriteRootNode(pugi::xml_node _rootNode, const Project& _project);
        static bool WriteDirectoryCollection(
            pugi::xml_node _rootNode,
            const char* _collectionNodeName,
            const char* _directoryNodeName,
            const Project::DirectoryCollection& _directories
        );
        static bool WritePlugins(pugi::xml_node _rootNode, const Project& _project);
        static bool WritePlugin(pugi::xml_node _parentNode, const Project::PluginDesc& _plugin, bool _writeType);
        static bool WriteEngineLogic(pugi::xml_node _rootNode, const Project& _project);
    };
} // namespace ego::engine
