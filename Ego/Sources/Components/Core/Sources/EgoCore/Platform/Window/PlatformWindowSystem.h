#pragma once

#include <functional>

#include "PlatformWindow.h"

namespace ego
{
    using PlatformWindowSystemQuitRequestedHandler = std::function<void()>;
    using PlatformWindowSystemWindowDestroyingHandler = std::function<void(const PlatformWindowPointer& _window)>;
    using PlatformWindowSystemWindowActivationHandler = std::function<void(const PlatformWindowPointer& _window, bool _isActive)>;
    using PlatformWindowSystemWindowSizeChangeHandler = std::function<void(const PlatformWindowPointer& _window, const PlatformWindowSize& _prevSize)>;

    class PlatformWindowSystem
    {
    public:
        PlatformWindowSystem() = default;
        virtual ~PlatformWindowSystem() = default;

        virtual bool init();
        virtual void release();

        virtual PlatformWindowPointer createWindow(const PlatformWindowDesc& _desc) = 0;
        virtual void processEvents() = 0;

        void setQuitRequestedHandler(const PlatformWindowSystemQuitRequestedHandler& _handler);
        void setWindowDestroyingHandler(const PlatformWindowSystemWindowDestroyingHandler& _handler);
        void setWindowActivationHandler(const PlatformWindowSystemWindowActivationHandler& _handler);
        void setWindowSizeChangeHandler(const PlatformWindowSystemWindowSizeChangeHandler& _handler);

        EGO_RTTI_VIRTUAL_BASE(PlatformWindowSystem);

    protected:
        void notifyQuitRequested() const;
        void notifyWindowDestroying(const PlatformWindowPointer& _window) const;
        void notifyWindowActivate(const PlatformWindowPointer& _window, bool _isActive) const;
        void notifyWindowSizeChange(const PlatformWindowPointer& _window, const PlatformWindowSize& _prevSize) const;

    private:
        PlatformWindowSystemQuitRequestedHandler m_quitRequestedHandler;
        PlatformWindowSystemWindowDestroyingHandler m_windowDestroyingHandler;
        PlatformWindowSystemWindowActivationHandler m_windowActivationHandler;
        PlatformWindowSystemWindowSizeChangeHandler m_windowSizeChangeHandler;
    };

    EGO_POINTER(PlatformWindowSystem);
} // namespace ego
