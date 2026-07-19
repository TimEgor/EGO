#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/Window.h"

#include "EgoEngine/Gui/Viewport/EngineGuiViewportBackend.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    class ApplicationWindowGuiViewportEventSource;
    class WindowGraphicPresenter;

    EGO_POINTER(ApplicationWindowGuiViewportEventSource);
    EGO_POINTER(WindowGraphicPresenter);

    class ApplicationWindowGuiViewportHost final : public engine::EngineViewportHost, public NonCopyable
    {
    public:
        ApplicationWindowGuiViewportHost() = default;
        ~ApplicationWindowGuiViewportHost() override;

        bool init(const ApplicationWindowPointer& _window, gui::ViewportRole _role);
        void release();

        void update();
        void beginClosing();

        gui::ViewportUpdateStatus getUpdateStatus() const;
        const gui::Size& getSize() const;
        void drainInput(gui::InputEventCollection& _input);

        engine::EngineViewportPrepareResult prepareForRender() override;
        bool resizeRenderTarget() override;
        GraphicPresenterPointer getGraphicPresenterPointer() const override;

    private:
        enum class State
        {
            Active,
            Closing,
            Failed
        };

        bool initGraphicPresenter();
        bool initGuiEventSource();

        ApplicationWindowPointer m_window = nullptr;
        gui::ViewportRole m_role = gui::ViewportRole::Primary;
        State m_state = State::Closing;
        gui::Size m_size = gui::SizeZero;

        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        WindowSize m_presenterSize = DefaultWindowSize;

        ApplicationWindowGuiViewportEventSourcePointer m_guiEventSource = nullptr;
    };

    EGO_POINTER(ApplicationWindowGuiViewportHost);
} // namespace ego::application
