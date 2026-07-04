#pragma once

#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoRuntime/Plugin/Plugin.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/EngineContext.h"

#include "EngineLogic.h"
#include "Plugin/EngineLogicPlugin.h"
#include "Project.h"

namespace ego::engine_framework
{
    class EngineFramework : public NonCopyable
    {
    public:
        struct InitData final
        {
            engine::Engine::InitData m_engineInitData;
            ProjectPointer m_project = nullptr;
        };

        EngineFramework() = default;
        ~EngineFramework() override
        {
            release();
        }

        bool init(const InitData& _initData);
        void release();

        void run();
        bool runFrame();

        engine::Engine& getEngine() const;
        const Project& getProject() const;
        EngineLogic& getCurrentEngineLogic() const;

    private:
        // Initialization
        bool initEngine(const InitData& _initData);
        bool initProject(const InitData& _initData);
        bool registerEngineLogicMainLoopJob();

        // Release
        void releaseEngine();
        void releaseProjectAssetFileSystems();

        // Project resources
        bool registerProjectAssetFileSystems();
        FileSystemPointer createProjectAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const;

        // Plugin loading
        bool loadProjectPlugins();
        bool loadPlugins(const Project::PluginCollection& _plugins);
        bool loadPlugin(const PluginPointer& _plugin);

        // Engine logic
        bool loadProjectEngineLogic();
        bool loadEngineLogic(const EngineLogicPluginPointer& _plugin);
        void updateCurrentEngineLogic(float _deltaTime);

        // Context state
        engine::EngineContextPointer m_engineContext = nullptr;

        ProjectPointer m_project = nullptr;
        std::vector<FileSystemPointer> m_projectAssetFileSystems;

        // Runtime state
        std::vector<PluginPointer> m_plugins;
        EngineLogicPluginPointer m_currentEngineLogicPlugin = nullptr;
        EngineLogicPointer m_currentEngineLogic = nullptr;
        JobDescriptorID m_updateEngineLogicJobID;
    };

    EGO_POINTER(EngineFramework);
    EGO_WEAK_POINTER(EngineFramework);
} // namespace ego::engine_framework
