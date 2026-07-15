#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "EgoCore/Clock.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoECS/Entity.h"

#include "EgoJob/JobGraph.h"

#include "EgoGui/GuiController.h"

#include "EgoPlugin/Catalog/PluginCatalog.h"

#include "Graphic/Presenter/GraphicPresenter.h"
#include "Graphic/Render/Render.h"
#include "Level/LevelController.h"
#include "Project/Project.h"
#include "FrameLogic.h"

namespace ego
{
    class FileSystem;
    class InputController;
    class JobController;
    class Plugin;
    class PluginController;
    class ResourceController;

    EGO_POINTER(FileSystem);
    EGO_POINTER(InputController);
    EGO_POINTER(JobController);
    EGO_POINTER(Plugin);
    EGO_POINTER(PluginController);
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::render
{
    class RenderPlugin;

    EGO_POINTER(RenderPlugin);
} // namespace ego::render

namespace ego::engine
{
    using EngineSessionID = uint32_t;
    inline constexpr EngineSessionID InvalidEngineSessionID = 0;

    class EngineLogic;
    class EngineLogicPlugin;

    EGO_POINTER(EngineLogic);
    EGO_POINTER(EngineLogicPlugin);

    class EngineSession final : public NonCopyable, public EnableSharedFromThis<EngineSession>
    {
    public:
        struct InitData final
        {
            ProjectPointer m_project = nullptr;
            FileName m_renderPluginModuleName;
            gui::GuiViewportDesc m_guiViewportDesc;
        };

        EngineSession() = default;
        ~EngineSession() override;

        bool init(const JobControllerPointer& _jobController, EngineSessionID _id, const InitData& _initData);
        void release();
        bool tick();

        EngineSessionID getID() const;

        LevelController& getLevelController();

        render::Render& getRender();

        bool setGraphicPresenter(const GraphicPresenterPointer& _graphicPresenter);
        void clearGraphicPresenter();

        void setRenderCameraEntity(ecs::Entity _cameraEntity);
        void clearRenderCameraEntity();

        gui::GuiController& getGuiController();
        gui::GuiControllerPointer getGuiControllerPointer() const;

        InputControllerPointer getInputControllerPointer() const;

    private:
        bool initProject(const ProjectPointer& _project);
        void releaseProject();

        FileSystemPointer getFileSystemPointer() const;
        PluginControllerPointer getPluginControllerPointer() const;
        ResourceControllerPointer getResourceControllerPointer() const;

        bool buildProjectPluginCatalog(const Project& _project);
        bool registerProjectAssetFileSystems(const Project& _project);
        void releaseProjectAssetFileSystems();
        FileSystemPointer createProjectAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const;

        bool loadProjectPlugins(const Project& _project);
        bool loadProjectPlugin(const Project::PluginDesc& _pluginDesc);
        bool loadProjectEngineLogicPlugin(const Project& _project);
        FileName resolveProjectPluginModuleName(const Project::PluginDesc& _pluginDesc) const;
        FileName resolvePluginModuleName(PluginType _pluginType) const;
        FileName resolvePluginModuleName(PluginType _pluginType, std::string_view _pluginName) const;

        bool initInputController();
        bool loadDefaultGuiFont(gui::GuiFontAtlasDesc& _fontAtlasDesc) const;
        bool initGuiController(const InitData& _initData);
        bool initRender(const InitData& _initData);
        bool initFrameLogic();
        void cleanResources();

        bool initEngineLogic();
        void releaseEngineLogic();
        bool registerEngineLogicFrameLogicJob();
        void unregisterEngineLogicFrameLogicJob();
        void updateEngineLogic();

        void beginFrame();
        void endFrame();
        float getDeltaTime() const;
        JobGraphReference getFrameLogicJobGraph();
        void renderFrame();
        void presentFrame();
        void prepareRenderFrame();

        using ProjectAssetFileSystemCollection = std::vector<FileSystemPointer>;
        using ProjectPluginCollection = std::vector<PluginPointer>;

        ProjectAssetFileSystemCollection m_projectAssetFileSystems;
        ProjectPluginCollection m_projectPlugins;
        PluginCatalog m_projectPluginCatalog;
        EngineLogicPluginPointer m_engineLogicPlugin = nullptr;

        EngineLogicPointer m_engineLogic = nullptr;
        JobDescriptorID m_updateEngineLogicJobID;

        LevelControllerPointer m_levelController = nullptr;
        JobControllerPointer m_jobController = nullptr;

        FrameLogic m_frameLogic;

        GraphicPresenterWeakPointer m_graphicPresenter;
        render::RenderPluginPointer m_renderPlugin = nullptr;
        render::RenderPointer m_render = nullptr;

        InputControllerPointer m_inputController = nullptr;
        gui::GuiControllerPointer m_guiController = nullptr;

        ClockTimePoint m_currentFrameTime;
        ClockTimePoint m_prevFrameStartTime;

        EngineSessionID m_id = InvalidEngineSessionID;
        ecs::Entity m_renderCameraEntity;

        float m_deltaTime = 0.0f;
    };

    EGO_POINTER(EngineSession);
    EGO_WEAK_POINTER(EngineSession);
} // namespace ego::engine
