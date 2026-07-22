#pragma once

#include <cstdint>

#include "EgoCore/Clock.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoECS/Entity.h"

#include "EgoJob/JobGraph.h"

#include "EgoGui/GuiController.h"
#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Theme/Theme.h"

#include "FrameLogic.h"
#include "Graphic/GraphicFrameController.h"
#include "Level/LevelController.h"
#include "Project/ProjectRuntime.h"

namespace ego
{
    class InputController;
    class JobController;
    class PluginController;
    class ResourceController;

    EGO_POINTER(InputController);
    EGO_POINTER(JobController);
    EGO_POINTER(PluginController);
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::render
{
    class Render;
} // namespace ego::render

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
            gui::FontAtlasDesc m_fontAtlasDesc;
            gui::Theme m_theme;
            gui::ViewportProviderPointer m_viewportProvider = nullptr;
        };

        struct SceneRenderOptions final
        {
            FileName m_pluginModuleName;
        };

        struct GuiRenderOptions final
        {
            FileName m_pluginModuleName;
        };

        struct InitData final
        {
            ProjectPointer m_project = nullptr;
            GuiOptions m_gui;
            SceneRenderOptions m_sceneRender;
            GuiRenderOptions m_guiRender;
            bool m_enableSceneRender = true;
            bool m_enablePresentation = false;
            bool m_enableGui = false;
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

        InputControllerPointer getInputControllerPointer() const;

    private:
        PluginControllerPointer getPluginControllerPointer() const;
        ResourceControllerPointer getResourceControllerPointer() const;

        bool initInputController();
        bool initGuiController(const GuiOptions& _guiOptions, bool _enableGui);
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
        JobGraphReference getFrameLogicJobGraph();
        void prepareGraphicFrame();

        ProjectRuntime m_projectRuntime;
        EngineLogicPointer m_engineLogic = nullptr;
        JobDescriptorID m_updateEngineLogicJobID;

        LevelControllerPointer m_levelController = nullptr;
        JobControllerPointer m_jobController = nullptr;

        FrameLogic m_frameLogic;
        GraphicFrameController m_graphicFrameController;

        InputControllerPointer m_inputController = nullptr;
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
