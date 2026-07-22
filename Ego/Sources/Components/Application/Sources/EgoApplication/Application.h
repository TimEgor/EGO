#pragma once

#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/Window.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

#include "EgoPlugin/Catalog/PluginCatalog.h"

namespace ego
{
    class DiagnosticSubsystem;
    class EventSubsystem;
    class PlatformSubsystem;
    class PluginSubsystem;
    class ResourceSubsystem;

    EGO_POINTER(DiagnosticSubsystem);
    EGO_POINTER(EventSubsystem);
    EGO_POINTER(PlatformSubsystem);
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
    class Application;
    class ApplicationSubsystem;
    class ApplicationProfiler;
    class PresenterProvider;
    struct ApplicationQuitRequestedEvent;

    EGO_POINTER(Application);
    EGO_WEAK_POINTER(Application);
    EGO_POINTER(ApplicationSubsystem);
    EGO_POINTER(ApplicationProfiler);
    EGO_POINTER(PresenterProvider);

    class Application final : public NonCopyable, public EnableSharedFromThis<Application>
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

        const PresenterProviderPointer& getPresenterProviderPointer() const;

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
        void unregisterPluginDirectory();
        bool registerGraphicResourceProvider();

        bool initWindowing();
        void releaseWindowing();
        void handleQuitRequested(const ApplicationQuitRequestedEvent& _event);

        subsystem::SubsystemRegistryPointer m_subsystemRegistry = nullptr;
        ApplicationSubsystemPointer m_applicationSubsystem = nullptr;
        DiagnosticSubsystemPointer m_diagnosticSubsystem = nullptr;
        PlatformSubsystemPointer m_platformSubsystem = nullptr;
        PluginSubsystemPointer m_pluginSubsystem = nullptr;
        PluginCatalog::RegistrationID m_pluginDirectoryRegistrationID = PluginCatalog::InvalidRegistrationID;
        EventSubsystemPointer m_eventSubsystem = nullptr;
        gpu::GraphicHardwareSubsystemPointer m_graphicHardwareSubsystem = nullptr;
        ResourceSubsystemPointer m_resourceSubsystem = nullptr;

        ApplicationProfilerPointer m_applicationProfiler = nullptr;

        PresenterProviderPointer m_presenterProvider = nullptr;
        EventCallbackID m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;

        bool m_isExitRequested = false;
    };
} // namespace ego::application
