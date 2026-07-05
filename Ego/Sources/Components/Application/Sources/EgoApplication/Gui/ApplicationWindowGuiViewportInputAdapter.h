#pragma once

#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/GuiViewportInputAdapter.h"

#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    class ApplicationWindowGuiViewportInputAdapter final : public gui::GuiViewportInputAdapter
    {
    public:
        ApplicationWindowGuiViewportInputAdapter() = default;
        ~ApplicationWindowGuiViewportInputAdapter() override = default;

        bool init(const ApplicationWindowPointer& _window, const GraphicPresenterPointer& _presenter);
        void release();

        bool adaptInputEvent(gui::GuiInputEvent& _event, const gui::GuiViewport& _viewport) const override;

        EGO_RTTI_VIRTUAL(ApplicationWindowGuiViewportInputAdapter, gui::GuiViewportInputAdapter);

    private:
        ApplicationWindowPointer m_window = nullptr;
        GraphicPresenterPointer m_presenter = nullptr;
    };

    EGO_POINTER(ApplicationWindowGuiViewportInputAdapter);
} // namespace ego::application
