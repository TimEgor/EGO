#pragma once

#include "EgoCore/Platform/FileSystem/FileSystem.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/GuiStyle.h"
#include "EgoGui/Platform/PlatformAdapter.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/GuiTexture.h"
#include "EgoGui/Rendering/RendererAdapter.h"
#include "EgoGui/Viewport/ViewportProvider.h"

struct ImGuiContext;

namespace ego::gui
{
    class Layer;

    class Backend final
    {
        friend class Layer;

    public:
        Backend();
        ~Backend();

        Backend(const Backend&) = delete;
        Backend& operator=(const Backend&) = delete;

        bool init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport);

        bool setFont(const FileContent& _content, float _size);
        bool setStyle(const GuiStyle& _style);

        bool beginFrame(float _deltaTime);
        bool endFrame(GuiRenderData& _renderData);
        void cancelFrame();

        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode);

        bool isInitialized() const;

    private:
        ImGuiContext& getContext() const;

        bool release();

        bool initializeContext(bool _enableMultiViewport);

        bool releaseContext();
        void resetState();

        RendererAdapter m_rendererAdapter;
        PlatformAdapter m_platformAdapter;

        ImGuiContext* m_context = nullptr;

        bool m_isFrameActive = false;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
