#pragma once

#include <shared_mutex>

#include "EgoCore/PlatformMacros.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoApplication/Window/Window.h"

namespace ego::win32
{
    class Win32Window final : public Window, public EnableSharedFromThis<Win32Window>
    {
    public:
        class Accessor final : public NonInstanceable
        {
            friend class Win32WindowSystem;

            static void OnWindowDestroying(Win32Window& _window);
            static void OnWindowTransformationStart(Win32Window& _window);
            static void OnWindowTransformationEnd(Win32Window& _window);
            static void OnWindowSizeUpdate(Win32Window& _window);
        };

        explicit Win32Window(HINSTANCE _instance);
        ~Win32Window() override;

        bool init(const WindowDesc& _desc) override;
        void release() override;

        bool isValid() const override;

        void show() override;
        void hide() override;
        bool isShown() const override;

        void* getNativeHandle() const override;

        bool isStable() const override;

        const WindowSize& getWindowSize() const override;
        const WindowSize& getClientAreaSize() const override;
        const WindowArea& getCutoutsArea() const override;

        HWND getHandle() const;

        EGO_RTTI_VIRTUAL(Win32Window, Window);

    private:
        void updateSizes();

        void invalidate();
        void setSizeStabilization(bool _state);

        mutable std::shared_mutex m_mutex;

        WindowSize m_windowSize = DefaultWindowSize;
        WindowSize m_clientAreaSize = DefaultWindowSize;
        WindowArea m_cutoutsArea = DefaultWindowArea;

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;

        bool m_isShown = false;
        bool m_isSizeStable = false;
    };

    EGO_POINTER(Win32Window);
    EGO_WEAK_POINTER(Win32Window);
} // namespace ego::win32
