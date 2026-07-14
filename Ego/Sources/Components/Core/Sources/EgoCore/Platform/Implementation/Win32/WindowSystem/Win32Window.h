#pragma once

#include <shared_mutex>

#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Window/Window.h"

namespace ego::win32
{
    class Win32WindowSystem;

    class Win32Window final : public Window, public EnableSharedFromThis<Win32Window>
    {
        friend class Win32WindowSystem;

    public:
        Win32Window(Win32WindowSystem& _windowSystem, HINSTANCE _instance);
        ~Win32Window() override;

        bool init(const WindowDesc& _desc) override;
        void release() override;

        bool isValid() const override;

        void show() override;
        void hide() override;
        bool isShown() const override;

        void* getNativeHandle() const override;

        bool isStable() const override;
        bool screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const override;

        const WindowSize& getWindowSize() const override;
        const WindowSize& getClientAreaSize() const override;
        const WindowArea& getCutoutsArea() const override;

        HWND getHandle() const;

        EGO_RTTI_VIRTUAL(Win32Window, Window);

    private:
        bool processWindowMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam, LRESULT& _result);

        void onWindowDestroying();
        void onWindowTransformationStart();
        void onWindowTransformationEnd();
        void onWindowSizeUpdate();
        void onWindowActivate(bool _isActive);
        void onWindowKeyboardInput(WindowKeyboardInputAction _action, WPARAM _wParam, LPARAM _lParam);
        void onWindowTextInput(WindowTextCodepoint _codepoint);

        void updateSizes();

        void invalidate();
        void setSizeStabilization(bool _state);

        mutable std::shared_mutex m_mutex;

        WindowSize m_windowSize = DefaultWindowSize;
        WindowSize m_clientAreaSize = DefaultWindowSize;
        WindowArea m_cutoutsArea = DefaultWindowArea;

        Win32WindowSystem& m_windowSystem;
        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;

        bool m_isShown = false;
        bool m_isSizeStable = false;
    };

    EGO_POINTER(Win32Window);
    EGO_WEAK_POINTER(Win32Window);
} // namespace ego::win32
