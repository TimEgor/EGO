#pragma once

#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/WindowTypes.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

namespace ego
{
    class DiagnosticSubsystem;
    class EventSubsystem;
    class PlatformSubsystem;
    class PlatformPluginSubsystem;
    class PluginSubsystem;
    class ResourceSubsystem;

    EGO_POINTER(DiagnosticSubsystem);
    EGO_POINTER(EventSubsystem);
    EGO_POINTER(PlatformSubsystem);
    EGO_POINTER(PlatformPluginSubsystem);
    EGO_POINTER(PluginSubsystem);
    EGO_POINTER(ResourceSubsystem);
} // namespace ego

namespace ego::gpu
{
    class GraphicHardwareSubsystem;

    EGO_POINTER(GraphicHardwareSubsystem);
} // namespace ego::gpu

namespace ego::application
{
    class ApplicationWindow;
    class ApplicationWindowController;
    class ApplicationProfiler;

    EGO_POINTER(ApplicationWindow);
    EGO_POINTER(ApplicationWindowController);
    EGO_POINTER(ApplicationProfiler);

    class Application final : public NonCopyable
    {
    public:
        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
            FileName m_pluginDirectory;
            FileName m_profilerPluginModuleName;
            FileName m_graphicHardwarePluginModuleName;
            bool m_enableGraphicHardware = false;
            bool m_enableWindowing = true;
        };

        Application();
        ~Application() override;

        bool init(const InitData& _initData);
        void release();

        ApplicationWindowPointer createWindow(const WindowDesc& _desc);

        void processWindowEvents();
        void updateInputDevices();

        void requestExit();
        bool isExitRequested() const;

    private:
        bool initSubsystems(const InitData& _initData);
        void releaseSubsystems();

        bool initSubsystemRegistry();
        void releaseSubsystemRegistry();

        bool registerSubsystem(const subsystem::SubsystemPointer& _subsystem);
        void releaseSubsystem(const subsystem::SubsystemPointer& _subsystem);

        bool registerPluginDirectory(const FileName& _pluginDirectory);
        bool registerGraphicResourceProvider();

        bool initWindowing();
        void releaseWindowing();

        subsystem::SubsystemRegistryPointer m_subsystemRegistry = nullptr;
        DiagnosticSubsystemPointer m_diagnosticSubsystem = nullptr;
        PlatformSubsystemPointer m_platformSubsystem = nullptr;
        PlatformPluginSubsystemPointer m_platformPluginSubsystem = nullptr;
        PluginSubsystemPointer m_pluginSubsystem = nullptr;
        EventSubsystemPointer m_eventSubsystem = nullptr;
        gpu::GraphicHardwareSubsystemPointer m_graphicHardwareSubsystem = nullptr;
        ResourceSubsystemPointer m_resourceSubsystem = nullptr;

        ApplicationProfilerPointer m_applicationProfiler = nullptr;

        ApplicationWindowControllerPointer m_windowController = nullptr;
        EventCallbackID m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;

        bool m_isExitRequested = false;
    };

    EGO_POINTER(Application);
} // namespace ego::application
