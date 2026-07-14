#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/WindowSystem.h"

#include "ApplicationWindow.h"

namespace ego
{
    class Platform;

    EGO_POINTER(Platform);
} // namespace ego

namespace ego::application
{
    class ApplicationWindowController
        : public WindowSystemEventListener
        , public EnableSharedFromThis<ApplicationWindowController>
        , public NonCopyable
    {
    public:
        ApplicationWindowController() = default;
        virtual ~ApplicationWindowController() = default;

        bool init(const PlatformPointer& _platform, const EventControllerPointer& _eventController);
        void release();

        ApplicationWindowPointer createApplicationWindow(const WindowDesc& _desc);
        bool containsApplicationWindow(const ApplicationWindowPointer& _window) const;
        void processWindowEvents();

        EGO_RTTI_VIRTUAL_BASE(ApplicationWindowController);

    private:
        using ApplicationWindowCollection = std::vector<ApplicationWindowPointer>;

        WindowSystem& getNativeWindowSystem() const;
        bool registerApplicationWindowEvents();
        void unregisterApplicationWindowEvents();
        bool registerWindowSystemEventListener();
        void unregisterWindowSystemEventListener();
        void releaseApplicationWindows();

        ApplicationWindowPointer findApplicationWindow(const ego::WindowPointer& _nativeWindow) const;
        void removeApplicationWindow(const ApplicationWindowPointer& _window);

        void onWindowSystemQuitRequested() override;
        void onWindowDestroying(const ego::WindowPointer& _nativeWindow) override;
        void onWindowActivation(const ego::WindowPointer& _nativeWindow, bool _isActive) override;
        void onWindowSizeChanged(const ego::WindowPointer& _nativeWindow, const WindowSize& _prevSize) override;
        void onWindowKeyboardInput(const ego::WindowPointer& _nativeWindow, const WindowKeyboardInputData& _inputData) override;
        void onWindowTextInput(const ego::WindowPointer& _nativeWindow, const WindowTextInputData& _inputData) override;

        void emitApplicationQuitRequested() const;
        void emitApplicationWindowDestroying(const ApplicationWindowPointer& _window) const;
        void emitApplicationWindowActivation(const ApplicationWindowPointer& _window, bool _isActive) const;
        void emitApplicationWindowSizeChanged(const ApplicationWindowPointer& _window, const WindowSize& _prevSize) const;
        void emitApplicationWindowKeyboardInput(const ApplicationWindowPointer& _window, const WindowKeyboardInputData& _inputData) const;
        void emitApplicationWindowTextInput(const ApplicationWindowPointer& _window, const WindowTextInputData& _inputData) const;

        PlatformPointer m_platform = nullptr;
        EventControllerPointer m_eventController = nullptr;
        ApplicationWindowCollection m_windows;
        bool m_isWindowSystemEventListenerRegistered = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ApplicationWindowController);
} // namespace ego::application
