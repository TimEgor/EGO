#pragma once

#include <unordered_map>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/Gui/Viewport/EngineGuiViewportBackend.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    class Application;
    class ApplicationWindowGuiViewportHost;

    EGO_POINTER(Application);
    EGO_POINTER(ApplicationWindowGuiViewportHost);

    class ApplicationGuiViewportSystem final : public engine::EngineGuiViewportBackend, public NonCopyable
    {
    public:
        ApplicationGuiViewportSystem() = default;
        ~ApplicationGuiViewportSystem() override;

        bool init(const ApplicationPointer& _application, const ApplicationWindowPointer& _primaryWindow);
        void release();

        bool createViewport(const gui::ViewportCreateRequest& _request) override;
        void destroyViewport(gui::ViewportID _viewportID) override;
        gui::ViewportUpdate pollViewport(gui::ViewportID _viewportID) override;

        engine::EngineViewportHostPointer findViewportHost(gui::ViewportID _viewportID) const override;

    private:
        using ViewportMap = std::unordered_map<gui::ViewportID, ApplicationWindowGuiViewportHostPointer>;

        ApplicationWindowGuiViewportHostPointer findViewport(gui::ViewportID _viewportID) const;

        static WindowDesc CreateSecondaryWindowDesc(const gui::ViewportCreateRequest& _request);

        ApplicationPointer m_application = nullptr;
        ApplicationWindowPointer m_primaryWindow = nullptr;
        ViewportMap m_viewports;
    };

    EGO_POINTER(ApplicationGuiViewportSystem);
} // namespace ego::application
