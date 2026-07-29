#pragma once

#include <cstdint>

#include "EgoCore/Clock.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoECS/Entity.h"

#include "EgoJob/JobGraph.h"

#include "EgoGui/GuiController.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

#include "FrameLogic.h"
#include "Graphic/GraphicFrameController.h"
#include "Level/LevelController.h"
#include "Project/ProjectRuntime.h"

namespace ego
{
    class JobController;
    class PluginController;

    EGO_POINTER(JobController);
    EGO_POINTER(PluginController);
} // namespace ego

namespace ego::render
{
    class Render;
} // namespace ego::render

namespace ego::application
{
    class ApplicationGuiViewportProvider;

    EGO_POINTER(ApplicationGuiViewportProvider);
} // namespace ego::application

namespace ego::engine
{
    using EngineSessionID = uint32_t;
    inline constexpr EngineSessionID InvalidEngineSessionID = 0;

    class EngineLogic;
    EGO_POINTER(EngineLogic);

    class EngineSession final : public NonCopyable, public EnableSharedFromThis<EngineSession>
    {
    public:
        struct GuiOptions final
        {
            FileName m_pluginModuleName;
            bool m_isEnabled = false;
        };

        struct SceneRenderOptions final
        {
            FileName m_pluginModuleName;
            bool m_isEnabled = true;
        };

        struct InitData final
        {
            ProjectPointer m_project = nullptr;
            application::Presentation m_mainPresentation;
            GuiOptions m_gui;
            SceneRenderOptions m_sceneRender;
        };

        EngineSession();
        ~EngineSession() override;

        bool init(const JobControllerPointer& _jobController, EngineSessionID _id, const InitData& _initData);
        void release();
        bool tick();

        EngineSessionID getID() const;

        LevelController& getLevelController();

        render::Render& getRender();

        void setRenderCameraEntity(ecs::Entity _cameraEntity);
        void clearRenderCameraEntity();

        gui::GuiControllerPointer getGuiControllerPointer() const;

    private:
        PluginControllerPointer getPluginControllerPointer() const;

        bool initGuiController(const application::Presentation& _mainPresentation);
        bool initGraphicFrameController(const InitData& _initData);
        bool initFrameLogic();

        bool initEngineLogic();
        void releaseEngineLogic();
        bool registerEngineLogicFrameLogicJob();
        void unregisterEngineLogicFrameLogicJob();
        void updateEngineLogic();

        void beginFrame();
        void endFrame();
        float getDeltaTime() const;
        JobGraphPointer getFrameLogicJobGraph();
        void prepareGraphicFrame();

        ProjectRuntime m_projectRuntime;
        EngineLogicPointer m_engineLogic = nullptr;
        JobDescriptorID m_updateEngineLogicJobID;

        LevelControllerPointer m_levelController = nullptr;
        JobControllerPointer m_jobController = nullptr;

        FrameLogic m_frameLogic;
        GraphicFrameController m_graphicFrameController;

        GraphicPresenterPointer m_scenePresenter = nullptr;
        application::ApplicationGuiViewportProviderPointer m_guiViewportProvider = nullptr;
        gui::GuiControllerPointer m_guiController = nullptr;

        ClockTimePoint m_currentFrameTime;
        ClockTimePoint m_prevFrameStartTime;

        EngineSessionID m_id = InvalidEngineSessionID;
        ecs::Entity m_renderCameraEntity;

        float m_deltaTime = 0.0f;
        bool m_isGuiEnabled = false;
    };

    EGO_POINTER(EngineSession);
    EGO_WEAK_POINTER(EngineSession);
} // namespace ego::engine
