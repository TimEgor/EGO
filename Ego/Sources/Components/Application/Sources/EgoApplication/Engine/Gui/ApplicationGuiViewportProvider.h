#pragma once

#include <unordered_map>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Viewport/ViewportProvider.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

namespace ego::application
{
    class ApplicationGuiViewport;

    class ApplicationGuiViewportProvider final : public gui::ViewportProvider, public NonCopyable
    {
    public:
        ApplicationGuiViewportProvider() = default;
        ~ApplicationGuiViewportProvider() override;

        bool init(const Presentation& _primaryPresentation);
        void release();

        bool createViewport(const gui::ViewportCreateRequest& _request) override;
        void destroyViewport(gui::ViewportID _viewportID) override;
        gui::ViewportUpdate pollViewport(gui::ViewportID _viewportID) override;

    private:
        using ViewportPointer = SharedPointer<ApplicationGuiViewport>;
        using ViewportMap = std::unordered_map<gui::ViewportID, ViewportPointer>;

        void releaseViewport(ViewportPointer& _viewport);

        static PresenterProviderPointer GetPresenterProvider();
        static PresentationDesc CreateViewportPresentationDesc(const gui::ViewportCreateRequest& _request);

        Presentation m_primaryPresentation;
        ViewportMap m_viewports;
    };

    EGO_POINTER(ApplicationGuiViewportProvider);
} // namespace ego::application
