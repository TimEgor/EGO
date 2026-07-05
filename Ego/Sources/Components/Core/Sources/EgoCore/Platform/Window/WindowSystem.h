#pragma once

#include <functional>

#include "Window.h"

namespace ego
{
    using WindowSystemQuitRequestedHandler = std::function<void()>;
    using WindowSystemWindowDestroyingHandler = std::function<void(const WindowPointer& _window)>;
    using WindowSystemWindowActivationHandler = std::function<void(const WindowPointer& _window, bool _isActive)>;
    using WindowSystemWindowSizeChangeHandler = std::function<void(const WindowPointer& _window, const WindowSize& _prevSize)>;

    class WindowSystem
    {
    public:
        WindowSystem() = default;
        virtual ~WindowSystem() = default;

        virtual bool init();
        virtual void release();

        virtual WindowPointer createWindow(const WindowDesc& _desc) = 0;
        virtual void processEvents() = 0;

        void setQuitRequestedHandler(const WindowSystemQuitRequestedHandler& _handler);
        void setWindowDestroyingHandler(const WindowSystemWindowDestroyingHandler& _handler);
        void setWindowActivationHandler(const WindowSystemWindowActivationHandler& _handler);
        void setWindowSizeChangeHandler(const WindowSystemWindowSizeChangeHandler& _handler);

        EGO_RTTI_VIRTUAL_BASE(WindowSystem);

    protected:
        void notifyQuitRequested() const;
        void notifyWindowDestroying(const WindowPointer& _window) const;
        void notifyWindowActivate(const WindowPointer& _window, bool _isActive) const;
        void notifyWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const;

    private:
        WindowSystemQuitRequestedHandler m_quitRequestedHandler;
        WindowSystemWindowDestroyingHandler m_windowDestroyingHandler;
        WindowSystemWindowActivationHandler m_windowActivationHandler;
        WindowSystemWindowSizeChangeHandler m_windowSizeChangeHandler;
    };

    EGO_POINTER(WindowSystem);
} // namespace ego
