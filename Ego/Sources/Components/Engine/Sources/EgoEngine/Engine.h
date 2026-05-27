#pragma once

#include "EgoCore/Clock.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Job/JobController.h"
#include "EgoCore/Job/JobGraph.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoECS/Entity.h"

#include "Event/EventController.h"
#include "Graphic/Presenter/WindowGraphicPresenter.h"
#include "Graphic/Render/DefaultRender.h"
#include "Graphic/RenderHardware/GraphicDevice.h"
#include "Graphic/RenderHardware/RenderHardwarePlugin.h"
#include "Level/LevelController.h"
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
}

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
            FileName m_renderHardwarePluginModuleName;
            PluginDirectoryCollection m_pluginDirectories;
        };

        Engine() = default;
        virtual ~Engine() = default;

        bool init(const InitData& _initData);
        void release();

        void run();
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

        const EventController& getEventController() const;
        EventController& getEventController();

        const JobController& getJobController() const;
        JobController& getJobController();

        const ResourceController& getResourceController() const;
        ResourceController& getResourceController();

        const LevelController& getLevelController() const;
        LevelController& getLevelController();

        const DefaultRender& getRender() const;
        DefaultRender& getRender();

        ecs::Entity getRenderCameraEntity() const;
        void setRenderCameraEntity(ecs::Entity _cameraEntity);
        void clearRenderCameraEntity();

        const PluginCatalog& getPluginCatalog() const;
        PluginCatalog& getPluginCatalog();

    protected:
        virtual JobGraphReference getMainLoopJobGraph();

        void renderFrame();

    private:
        bool initPluginController();
        bool initPlatform(const InitData& _initData);
        bool initPluginCatalog(const InitData& _initData);
        bool initGraphicDevice(const InitData& _initData);

        void beginFrame();
        void endFrame();
        bool prepareMainWindowPresenter();

        EnginePluginControllerPointer m_enginePluginController = nullptr;

        EventControllerPointer m_eventController = nullptr;
        JobControllerPointer m_jobController = nullptr;
        ResourceControllerPointer m_resourceController = nullptr;
        LevelControllerPointer m_levelController = nullptr;

        PluginCatalog m_pluginCatalog;

        PlatformPointer m_platform = nullptr;
        GraphicDevicePointer m_graphicDevice = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        DefaultRenderPointer m_render = nullptr;

        PlatformPluginPointer m_platformPlugin = nullptr;
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
}
