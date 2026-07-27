#pragma once

#include <memory>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Viewport/ViewportTypes.h"

namespace ego::gui
{
    class Viewport;
    class ViewportManager;
    class ViewportProvider;
    class Window;

    EGO_POINTER(Viewport);
    EGO_POINTER(ViewportProvider);
    EGO_POINTER(Window);

    class GuiController final : public NonCopyable
    {
    public:
        struct InitData final
        {
            FontAtlasDesc m_fontAtlasDesc;
            Theme m_theme = Theme::GetDefault();
            ViewportProviderPointer m_viewportProvider = nullptr;
            bool m_enableMultiViewport = false;
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
        ViewportPointer findViewport(const WindowPointer& _window) const;

        void setMultiViewportEnabled(bool _isEnabled);
        bool isMultiViewportEnabled() const;

        void setTheme(const Theme& _theme);
        ThemePointer getTheme() const;

    private:
        class VisualOperationScope final : public NonCopyable
        {
        public:
            explicit VisualOperationScope(GuiController& _controller);
            ~VisualOperationScope() override;

        private:
            GuiController& m_controller;
        };

        void releaseState();
        bool ensureVisualOperationInactive() const;
        void applyTheme(ThemePointer _theme);

        std::unique_ptr<ViewportManager> m_viewportManager;
        FontAtlasPointer m_fontAtlas = nullptr;
        ThemePointer m_theme = nullptr;
        bool m_isVisualOperationActive = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
