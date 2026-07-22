#pragma once

#include <unordered_map>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Input/Input.h"
#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Theme/Theme.h"

#include "EgoGui/Viewport/Viewport.h"
#include "EgoGui/Viewport/ViewportProvider.h"

namespace ego::gui
{
    class GuiController final
        : public NonCopyable
    {
    public:
        struct InitData final
        {
            FontAtlasDesc m_fontAtlasDesc;
            Theme m_theme = Theme::GetDefault();
            ViewportProviderPointer m_viewportProvider = nullptr;
        };

        GuiController();
        ~GuiController() override;

        bool init(const InitData& _initData);
        void release();

        void update();
        GuiRenderData buildFrame();

        bool isInitialized() const;

        ViewportPointer createViewport(const ViewportDesc& _desc);
        bool destroyViewport(const ViewportPointer& _viewport);

        ViewportPointer getPrimaryViewport() const;

        void setTheme(const Theme& _theme);
        ThemePointer getTheme() const;

    private:
        class VisualOperationScope final
            : public NonCopyable
        {
        public:
            explicit VisualOperationScope(GuiController& _controller);
            ~VisualOperationScope() override;

        private:
            GuiController& m_controller;
        };

        static constexpr ViewportID FirstViewportID = 1;

        using ViewportMap = std::unordered_map<ViewportID, ViewportPointer>;

        ViewportPointer createViewport(ViewportRole _role, const ViewportDesc& _desc);
        ViewportPointer createViewport(const ViewportCreateRequest& _request);
        ViewportPointer findViewport(ViewportID _viewportID) const;
        ViewportID prepareNewViewportID();
        bool ensureVisualOperationInactive() const;
        void applyTheme(ThemePointer _theme);

        ViewportMap m_viewports;
        ViewportID m_primaryViewportID = InvalidViewportID;
        ViewportID m_nextViewportID = FirstViewportID;
        ViewportProviderPointer m_viewportProvider = nullptr;
        FontAtlasPointer m_fontAtlas = nullptr;
        ThemePointer m_theme = nullptr;
        bool m_isVisualOperationActive = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
