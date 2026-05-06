#pragma once

#include "EgoCore/Clock.h"
#include "EgoCore/Patterns/Singleton.h"

#include "Platform/PlatformPlugin.h"
#include "RenderHardware/RenderHardwarePlugin.h"

namespace ego
{
    class PluginController;
    class EventController;
    class Platform;

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

        const EnginePluginController& getPluginController() const;
        EnginePluginController& getPluginController();

    private:
        void beginFrame();
        void endFrame();

        PluginController* m_pluginController = nullptr;
        EnginePluginController* m_enginePluginController = nullptr;

        EventController* m_eventController = nullptr;

        Platform* m_platform = nullptr;
        gpu::GraphicDevice* m_graphicDevice = nullptr;

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

    class EngineCore final : public Singleton<EngineCore>
    {
    public:
        EngineCore() = default;

        Engine& getEngine() const;
        void init(Engine* _engine);

    private:
        Engine* m_engine;
    };

    Engine& GetEngine();
}
