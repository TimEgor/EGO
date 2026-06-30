#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEngine/Event/EventController.h"

#include "Plugin/ApplicationPluginController.h"
#include "Window/WindowSystem.h"
#include "Window/WindowSystemPlugin.h"

namespace ego::application
{
    class Application
    {
    public:
        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
            FileName m_windowSystemPluginModuleName;
        };

        Application() = default;
        virtual ~Application();

        bool init(const InitData& _initData);
        void release();

        const EventController& getEventController() const;
        EventController& getEventController();

        const WindowSystem& getWindowSystem() const;
        WindowSystem& getWindowSystem();

    private:
        bool initPluginController();
        bool initEventController();
        bool initWindowSystem(const InitData& _initData);

        ApplicationPluginControllerPointer m_pluginController = nullptr;
        EventControllerPointer m_eventController = nullptr;
        WindowSystemPluginPointer m_windowSystemPlugin = nullptr;
        WindowSystemPointer m_windowSystem = nullptr;
    };

    EGO_POINTER(Application);

    class ApplicationCore final : public Singleton<ApplicationCore>
    {
    public:
        ApplicationCore() = default;

        bool init(const ApplicationPointer& _application);
        void release();

        ApplicationPointer getApplication() const;

    private:
        ApplicationPointer m_application = nullptr;
    };

    Application& GetApplication();
} // namespace ego::application
