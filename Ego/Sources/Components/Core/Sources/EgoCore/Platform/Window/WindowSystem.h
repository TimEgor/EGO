#pragma once

#include <vector>

#include "Window.h"

namespace ego
{
    class WindowSystemEventListener
    {
    public:
        WindowSystemEventListener() = default;
        virtual ~WindowSystemEventListener() = default;

        virtual void onWindowSystemQuitRequested() = 0;
        virtual void onWindowDestroying(const WindowPointer& _window) = 0;
        virtual void onWindowActivation(const WindowPointer& _window, bool _isActive) = 0;
        virtual void onWindowSizeChanged(const WindowPointer& _window, const WindowSize& _prevSize) = 0;
        virtual void onWindowKeyboardInput(const WindowPointer& _window, const WindowKeyboardInputData& _inputData) = 0;
        virtual void onWindowTextInput(const WindowPointer& _window, const WindowTextInputData& _inputData) = 0;
    };

    EGO_POINTER(WindowSystemEventListener);
    EGO_WEAK_POINTER(WindowSystemEventListener);

    class WindowSystem
    {
    public:
        WindowSystem() = default;
        virtual ~WindowSystem() = default;

        virtual bool init();
        virtual void release();

        virtual WindowPointer createWindow(const WindowDesc& _desc) = 0;
        virtual void processEvents() = 0;

        bool registerEventListener(const WindowSystemEventListenerPointer& _listener);
        void unregisterEventListener(const WindowSystemEventListenerPointer& _listener);

        EGO_RTTI_VIRTUAL_BASE(WindowSystem);

    protected:
        void notifyQuitRequested() const;
        void notifyWindowDestroying(const WindowPointer& _window) const;
        void notifyWindowActivate(const WindowPointer& _window, bool _isActive) const;
        void notifyWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const;
        void notifyWindowKeyboardInput(const WindowPointer& _window, const WindowKeyboardInputData& _inputData) const;
        void notifyWindowTextInput(const WindowPointer& _window, const WindowTextInputData& _inputData) const;

    private:
        using EventListenerCollection = std::vector<WindowSystemEventListenerWeakPointer>;

        std::vector<WindowSystemEventListenerPointer> collectEventListeners() const;

        EventListenerCollection m_eventListeners;
    };

    EGO_POINTER(WindowSystem);
} // namespace ego
