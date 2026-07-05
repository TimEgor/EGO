#pragma once

#include "EgoCore/Clock.h"

#include "EgoECS/Entity.h"

#include "EgoRuntime/Job/JobController.h"
#include "EgoRuntime/Job/JobGraph.h"

#include "EgoGui/GuiController.h"

#include "Graphic/Presenter/GraphicPresenter.h"
#include "Graphic/Render/RenderPlugin.h"
#include "Level/LevelController.h"
#include "MainLoop.h"

namespace ego
{
    class EventController;
    class JobController;
} // namespace ego

namespace ego::engine
{
    class Engine
    {
    public:
        struct InitData final
        {
            ego::render::RenderPluginPointer m_renderPlugin = nullptr;
            GraphicPresenterPointer m_graphicPresenter = nullptr;
            gui::GuiViewportDesc m_guiViewportDesc;
            uint32_t m_jobThreadCount = 0;
            const char* m_jobThreadName = "EgoJob";
        };

        Engine() = default;
        virtual ~Engine() = default;

        bool init(const InitData& _initData);
        void release();

        void run();
        bool runFrame();
        void cleanResources();

        void stop();
        void pause();
        void unpause();

        uint32_t getCurrentFrameNum() const;

        float getDeltaTime() const;
        void setTimeScale(float _scale);

        bool isStopped() const;
        bool isPaused() const;

        const LevelController& getLevelController() const;
        LevelController& getLevelController();

        const ego::render::Render& getRender() const;
        ego::render::Render& getRender();

        ecs::Entity getRenderCameraEntity() const;
        void setRenderCameraEntity(ecs::Entity _cameraEntity);
        void clearRenderCameraEntity();

        const gui::GuiController& getGuiController() const;
        gui::GuiController& getGuiController();
        gui::GuiControllerPointer getGuiControllerPointer() const;

        const MainLoop& getMainLoop() const;
        MainLoop& getMainLoop();

    private:
        bool initGraphicPresenter(const InitData& _initData);
        bool loadDefaultGuiFont(gui::GuiFontAtlasDesc& _fontAtlasDesc) const;
        bool initGuiController(const InitData& _initData);
        bool initRender(const InitData& _initData);
        bool initJobController(const InitData& _initData);
        bool initMainLoop();
        void syncPresenterTargetResolution();

        void releaseJobController();

        void beginFrame();
        void endFrame();
        JobGraphReference getMainLoopJobGraph();
        void renderFrame();
        void presentFrame();
        void prepareRenderFrame();

        LevelControllerPointer m_levelController = nullptr;
        JobControllerPointer m_jobController = nullptr;

        MainLoop m_mainLoop;

        GraphicPresenterPointer m_graphicPresenter = nullptr;
        ego::render::RenderPointer m_render = nullptr;

        ego::render::RenderPluginPointer m_renderPlugin = nullptr;
        gui::GuiControllerPointer m_guiController = nullptr;

        ClockTimePoint m_startTime;
        ClockTimePoint m_currentFrameTime;
        ClockTimePoint m_prevFrameStartTime;

        uint32_t m_currentFrame = 0;
        ecs::Entity m_renderCameraEntity;

        float m_deltaTime = 0.0f;
        float m_timeScale = 1.0f;

        bool m_isStopped = false;
        bool m_isPaused = false;
    };

    EGO_POINTER(Engine);
    EGO_WEAK_POINTER(Engine);

    Engine& GetEngine();
} // namespace ego::engine
