#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/Window.h"

#include "EgoGui/Viewport/ViewportBackend.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    class ApplicationWindowGuiViewportEventSource;

    EGO_POINTER(ApplicationWindowGuiViewportEventSource);

    class ApplicationWindowGuiViewportHost final : public NonCopyable
    {
    public:
        ApplicationWindowGuiViewportHost() = default;
        ~ApplicationWindowGuiViewportHost() override;

        bool init(const ApplicationWindowPointer& _window);
        void release();

        void update();
        void beginClosing();

        gui::ViewportUpdateStatus getUpdateStatus() const;
        const gui::Size& getSize() const;
        void drainInput(gui::InputEventCollection& _input);

    private:
        enum class State
        {
            Active,
            Closing
        };

        bool initGuiEventSource();

        ApplicationWindowPointer m_window = nullptr;
        State m_state = State::Closing;
        gui::Size m_size = gui::SizeZero;

        ApplicationWindowGuiViewportEventSourcePointer m_guiEventSource = nullptr;
    };

    EGO_POINTER(ApplicationWindowGuiViewportHost);
} // namespace ego::application
