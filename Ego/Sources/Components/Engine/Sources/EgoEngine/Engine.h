#pragma once

#include "EgoCore/Clock.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Job/JobController.h"
#include "EgoCore/Job/JobGraph.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoECS/Entity.h"

#include "Event/EventController.h"
#include "Graphic/Presenter/GraphicPresenter.h"
#include "Graphic/Render/RenderDeviceContext.h"
#include "Graphic/Render/RenderPlugin.h"
#include "Graphic/RenderHardware/GraphicDevice.h"
#include "Graphic/RenderHardware/RenderHardwarePlugin.h"
#include "Level/LevelController.h"
#include "MainLoop.h"
#include "Platform/Platform.h"
#include "Platform/PlatformPlugin.h"
#include "Plugin/EnginePluginController.h"
#include "Plugin/PluginCatalog.h"
#include "Resources/Resource/ResourceController.h"

#include <vector>

namespace ego
{
    class EventController;
    class Platform;
    class JobController;
    class ResourceController;
    class GraphicDevice;
} // namespace ego

namespace ego::engine
{
    class Engine
    {
    public:
        struct InitData final
        {
            using PluginDirectoryCollection = std::vector<FileName>;

            void* m_nativeInstanceHandle = nullptr;
            FileName m_platformPluginModuleName;
            FileName m_renderPluginModuleName;
            FileName m_renderHardwarePluginModuleName;
            GraphicPresenterPointer m_graphicPresenter = nullptr;
            PluginDirectoryCollection m_pluginDirectories;
        };

        Engine() = default;
        virtual ~Engine() = default;

        bool init(const InitData& _initData);
        void release();

        void run();
        bool runFrame();
        void completeRun();
        void stop();
        void pause();
        void unpause();

        uint32_t getCurrentFrameNum() const;

        float getDeltaTime() const;
        void setTimeScale(float _scale);

        bool isStopped() const;
        bool isPaused() const;

        const Platform& getPlatform() const;
        Platform& getPlatform();
        const GraphicDevice& getGraphicDevice() const;
        GraphicDevice& getGraphicDevice();
        const ego::render::RenderDeviceContext& getRenderDeviceContext() const;

        const EventController& getEventController() const;
        EventController& getEventController();

        const JobController& getJobController() const;
        JobController& getJobController();

        const ResourceController& getResourceController() const;
        ResourceController& getResourceController();

        const LevelController& getLevelController() const;
        LevelController& getLevelController();

        const ego::render::Render& getRender() const;
        ego::render::Render& getRender();

        ecs::Entity getRenderCameraEntity() const;
        void setRenderCameraEntity(ecs::Entity _cameraEntity);
        void clearRenderCameraEntity();

        const PluginCatalog& getPluginCatalog() const;
        PluginCatalog& getPluginCatalog();

        const MainLoop& getMainLoop() const;
        MainLoop& getMainLoop();

    private:
        bool initPluginController();
        bool initPlatform(const InitData& _initData);
        bool initPluginCatalog(const InitData& _initData);
        bool initGraphicDevice(const InitData& _initData);
        bool initGraphicPresenter(const InitData& _initData);
        bool initRender(const InitData& _initData);
        bool initMainLoop();

        void beginFrame();
        void endFrame();
        JobGraphReference getMainLoopJobGraph();
        void renderFrame();
        void presentFrame();
        void prepareRenderFrame();

        EnginePluginControllerPointer m_enginePluginController = nullptr;

        EventControllerPointer m_eventController = nullptr;
        JobControllerPointer m_jobController = nullptr;
        ResourceControllerPointer m_resourceController = nullptr;
        LevelControllerPointer m_levelController = nullptr;

        PluginCatalog m_pluginCatalog;
        MainLoop m_mainLoop;

        PlatformPointer m_platform = nullptr;
        GraphicDevicePointer m_graphicDevice = nullptr;
        ego::render::RenderDeviceContext m_renderDeviceContext;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        ego::render::RenderPointer m_render = nullptr;

        PlatformPluginPointer m_platformPlugin = nullptr;
        ego::render::RenderPluginPointer m_renderPlugin = nullptr;
        RenderHardwarePluginPointer m_renderHardwarePlugin = nullptr;

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

    class EngineCore final : public Singleton<EngineCore>
    {
    public:
        EngineCore() = default;

        EnginePointer getEngine() const;
        void init(const EnginePointer& _engine);
        void release();

    private:
        EnginePointer m_engine = nullptr;
    };

    Engine& GetEngine();
} // namespace ego::engine
