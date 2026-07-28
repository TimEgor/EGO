#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/GuiTexture.h"
#include "EgoGui/Viewport/ViewportProvider.h"

namespace ego::gui
{
    class GuiBackend : public NonCopyable
    {
    public:
        ~GuiBackend() override = default;

        virtual bool init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport) = 0;
        virtual bool release() = 0;

        virtual bool beginFrame(float _deltaTime) = 0;
        virtual bool endFrame(GuiRenderData& _renderData) = 0;
        virtual void cancelFrame() = 0;

        virtual GuiFrameTextureID bindTexture(const gpu::TextureViewReference& _textureView, TextureSamplingMode _samplingMode) = 0;

        virtual bool isInitialized() const = 0;
    };
} // namespace ego::gui
