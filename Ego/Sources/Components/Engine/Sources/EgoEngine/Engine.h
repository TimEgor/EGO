#pragma once

#include "EgoCore/Clock.h"
#include "EgoCore/Job/JobController.h"
#include "EgoCore/Patterns/Singleton.h"

#include "EgoPlugin/PluginController.h"

#include "Event/EventController.h"
#include "Platform/PlatformPlugin.h"
#include "Plugin/EnginePluginController.h"
#include "Graphic/Presenter/WindowGraphicPresenter.h"
#include "Graphic/Render/DefaultRender.h"
#include "Graphic/RenderHardware/RenderHardwarePlugin.h"
#include "Resources/Resource/ResourceController.h"

namespace ego
{
    class PluginController;
    class EventController;
    class Platform;
    class JobController;
    class FileSystem;
    class ResourceController;

    namespace gpu
    {
        class GraphicDevice;
    }
}

namespace ego::engine
{
    class EnginePluginController;

    struct EngineInitData final
    {
        void* m_nativeInstanceHandle = nullptr;
        FileName m_platformPluginModuleName;
        FileName m_renderHardwarePluginModuleName;
    };

    class Engine final
    {
    public:
        Engine() = default;

        bool init(const EngineInitData& _initData);
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
        const gpu::GraphicDevice& getGraphicDevice() const;
        gpu::GraphicDevice& getGraphicDevice();

        const EventController& getEventController() const;
        EventController& getEventController();

        const JobController& getJobController() const;
        JobController& getJobController();

        const ResourceController& getResourceController() const;
        ResourceController& getResourceController();

        const DefaultRender& getRender() const;
        DefaultRender& getRender();

        const EnginePluginController& getPluginController() const;
        EnginePluginController& getPluginController();

    private:
        void beginFrame();
        void endFrame();
        bool prepareMainWindowPresenter();
        void renderFrame();

        EnginePluginControllerPointer m_enginePluginController = nullptr;

        EventControllerPointer m_eventController = nullptr;
        JobControllerPointer m_jobController = nullptr;
        ResourceControllerPointer m_resourceController = nullptr;

        PlatformPointer m_platform = nullptr;
        gpu::GraphicDeviceReference m_graphicDevice = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        DefaultRenderPointer m_render = nullptr;

        PlatformPluginPointer m_platformPlugin = nullptr;
        RenderHardwarePluginPointer m_renderHardwarePlugin = nullptr;

        ClockTimePoint m_startTime;
        ClockTimePoint m_currentFrameTime;
        ClockTimePoint m_prevFrameStartTime;

        uint32_t m_currentFrame = 0;

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
