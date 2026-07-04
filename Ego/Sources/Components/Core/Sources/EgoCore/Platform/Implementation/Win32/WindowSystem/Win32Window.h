#pragma once

#include <shared_mutex>

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Window/PlatformWindow.h"

namespace ego::win32
{
    class Win32WindowSystem;

    class Win32Window final : public PlatformWindow, public EnableSharedFromThis<Win32Window>
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

        Win32Window(Win32WindowSystem& _windowSystem, HINSTANCE _instance);
        ~Win32Window() override;

        bool init(const PlatformWindowDesc& _desc) override;
        void release() override;

        bool isValid() const override;

        void show() override;
        void hide() override;
        bool isShown() const override;

        void* getNativeHandle() const override;

        bool isStable() const override;

        const PlatformWindowSize& getWindowSize() const override;
        const PlatformWindowSize& getClientAreaSize() const override;
        const PlatformWindowArea& getCutoutsArea() const override;

        HWND getHandle() const;
        Win32WindowSystem& getWindowSystem() const;

        EGO_RTTI_VIRTUAL(Win32Window, PlatformWindow);

    private:
        void updateSizes();

        void invalidate();
        void setSizeStabilization(bool _state);

        mutable std::shared_mutex m_mutex;

        PlatformWindowSize m_windowSize = DefaultPlatformWindowSize;
        PlatformWindowSize m_clientAreaSize = DefaultPlatformWindowSize;
        PlatformWindowArea m_cutoutsArea = DefaultPlatformWindowArea;

        Win32WindowSystem& m_windowSystem;
        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;

        bool m_isShown = false;
        bool m_isSizeStable = false;
    };

    EGO_POINTER(Win32Window);
    EGO_WEAK_POINTER(Win32Window);
} // namespace ego::win32
