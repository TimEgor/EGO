#pragma once

#include <memory>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Implementation/ImGui/Platform/ImGuiPlatformInput.h"
#include "EgoGui/Viewport/ViewportProvider.h"

struct ImDrawData;
struct ImGuiContext;
struct ImGuiViewport;
struct ImVec2;

namespace ego::gui
{
    struct ImGuiPlatformRenderTarget final
    {
        ImDrawData* m_drawData = nullptr;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
    };

    using ImGuiPlatformRenderTargetCollection = std::vector<ImGuiPlatformRenderTarget>;

    class ImGuiPlatformAdapter final : public NonCopyable
    {
    public:
        ~ImGuiPlatformAdapter() override;

        bool init(const ViewportProviderPointer& _provider);
        void release();

        bool beginFrame();
        bool endFrame();

        ImGuiPlatformRenderTargetCollection getRenderTargets() const;

    private:
        struct ViewportData;
        struct ViewportPollResult;

        using ViewportDataOwner = std::unique_ptr<ViewportData>;

        ViewportPollResult pollViewport(ImGuiViewport& _viewport, const ViewportData& _data);
        void destroyViewport(ImGuiViewport& _viewport);
        void updatePrimaryDisplay();
        void markViewportFailure(ImGuiViewport& _viewport);
        ViewportID allocateViewportID();

        static ImGuiPlatformAdapter* GetCurrentAdapter();
        static ViewportData* GetViewportData(ImGuiViewport* _viewport);
        static const ViewportData* GetViewportData(const ImGuiViewport* _viewport);
        static ViewportDataOwner TakeViewportData(ImGuiViewport& _viewport);

        static void PlatformCreateWindow(ImGuiViewport* _viewport);
        static void PlatformDestroyWindow(ImGuiViewport* _viewport);
        static void PlatformShowWindow(ImGuiViewport* _viewport);
        static void PlatformSetWindowPosition(ImGuiViewport* _viewport, ImVec2 _position);
        static ImVec2 PlatformGetWindowPosition(ImGuiViewport* _viewport);
        static void PlatformSetWindowSize(ImGuiViewport* _viewport, ImVec2 _size);
        static ImVec2 PlatformGetWindowSize(ImGuiViewport* _viewport);
        static ImVec2 PlatformGetWindowFramebufferScale(ImGuiViewport* _viewport);
        static void PlatformSetWindowFocus(ImGuiViewport* _viewport);
        static bool PlatformGetWindowFocus(ImGuiViewport* _viewport);
        static void PlatformSetWindowTitle(ImGuiViewport* _viewport, const char* _title);
        static void PlatformSetWindowAlpha(ImGuiViewport* _viewport, float _alpha);
        static void PlatformUpdateWindow(ImGuiViewport* _viewport);

        static constexpr ViewportID PrimaryViewportID = 1;

        ViewportProviderPointer m_provider = nullptr;
        ImGuiPlatformInput m_input;
        ImGuiContext* m_context = nullptr;
        ViewportID m_primaryViewportID = InvalidViewportID;
        ViewportID m_nextViewportID = InvalidViewportID;
        bool m_hasFatalPlatformError = false;
    };
} // namespace ego::gui
