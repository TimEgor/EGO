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

        bool init(const ApplicationWindowPointer& _window, gui::GuiViewportRole _role);
        void release();

        void update();
        void beginClosing();

        bool isCloseRequested() const;
        const gui::GuiSize& getSize() const;
        void drainEvents(gui::GuiViewportEventCollection& _events);

        engine::EngineViewportPrepareResult prepareForRender() override;
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
        gui::GuiViewportRole m_role = gui::GuiViewportRole::Primary;
        State m_state = State::Closing;
        gui::GuiSize m_size = gui::GuiSizeZero;

        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        WindowSize m_presenterSize = DefaultWindowSize;

        ApplicationWindowGuiViewportEventSourcePointer m_guiEventSource = nullptr;
    };

    EGO_POINTER(ApplicationWindowGuiViewportHost);
} // namespace ego::application
