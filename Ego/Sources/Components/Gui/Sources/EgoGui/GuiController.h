#pragma once

#include <unordered_map>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Input/GuiInput.h"
#include "EgoGui/Rendering/GuiFrame.h"
#include "EgoGui/Rendering/GuiFontAtlas.h"

#include "EgoGui/Viewport/GuiViewport.h"
#include "EgoGui/Viewport/GuiViewportBackend.h"

namespace ego::gui
{
    class GuiController final
    {
    public:
        struct InitData final
        {
            GuiFontAtlasDesc m_fontAtlasDesc;
            GuiViewportDesc m_primaryViewportDesc;
            GuiViewportBackendPointer m_viewportBackend = nullptr;
        };

        GuiController() = default;
        ~GuiController();

        bool init(const InitData& _initData);
        void release();

        GuiViewportID createViewport(const GuiViewportDesc& _desc);
        bool destroyViewport(GuiViewportID _viewportID);

        GuiViewportPointer findViewport(GuiViewportID _viewportID) const;

        GuiViewportPointer getPrimaryViewport() const;

        void update();
        GuiFrame buildFrame();

        GuiEventResult processEvent(GuiViewportID _viewportID, const GuiInputEvent& _event);
        bool isInitialized() const;

        EGO_RTTI_VIRTUAL_BASE(GuiController);

    private:
        static constexpr GuiViewportID FirstViewportID = 1;

        using ViewportMap = std::unordered_map<GuiViewportID, GuiViewportPointer>;

        GuiViewportID createViewport(GuiViewportRole _role, const GuiViewportDesc& _desc);
        GuiViewportID createViewport(const GuiViewportCreateRequest& _request);
        GuiViewportID prepareNewViewportID();

        ViewportMap m_viewports;
        GuiViewportID m_primaryViewportID = InvalidGuiViewportID;
        GuiViewportID m_nextViewportID = FirstViewportID;
        GuiViewportBackendPointer m_viewportBackend = nullptr;
        GuiFontAtlasPointer m_fontAtlas = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
