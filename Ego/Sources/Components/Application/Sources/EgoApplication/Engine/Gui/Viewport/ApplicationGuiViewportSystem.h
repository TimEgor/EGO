#pragma once

#include <unordered_map>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Viewport/ViewportBackend.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    class Application;
    class ApplicationWindowGuiViewportHost;

    EGO_POINTER(Application);
    EGO_POINTER(ApplicationWindowGuiViewportHost);

    class ApplicationGuiViewportSystem final : public gui::ViewportBackend, public NonCopyable
    {
    public:
        ApplicationGuiViewportSystem() = default;
        ~ApplicationGuiViewportSystem() override;

        bool init(
            const ApplicationPointer& _application,
            const ApplicationWindowPointer& _primaryWindow,
            const GraphicPresenterPointer& _primaryGraphicPresenter);
        void release();

        bool createViewport(const gui::ViewportCreateRequest& _request) override;
        void destroyViewport(gui::ViewportID _viewportID) override;
        gui::ViewportUpdate pollViewport(gui::ViewportID _viewportID) override;

    private:
        struct ViewportEntry final
        {
            ApplicationWindowGuiViewportHostPointer m_guiHost = nullptr;
            GraphicPresenterPointer m_graphicPresenter = nullptr;
        };

        using ViewportMap = std::unordered_map<gui::ViewportID, ViewportEntry>;

        ApplicationWindowGuiViewportHostPointer findViewport(gui::ViewportID _viewportID) const;
        GraphicPresenterPointer findGraphicPresenter(gui::ViewportID _viewportID) const;

        static WindowDesc CreateSecondaryWindowDesc(const gui::ViewportCreateRequest& _request);

        ApplicationPointer m_application = nullptr;
        ApplicationWindowPointer m_primaryWindow = nullptr;
        GraphicPresenterPointer m_primaryGraphicPresenter = nullptr;
        ViewportMap m_viewports;
    };

    EGO_POINTER(ApplicationGuiViewportSystem);
} // namespace ego::application
