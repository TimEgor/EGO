#pragma once

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

namespace ego
{
    class TextureGraphicPresenter final : public GraphicPresenter
    {
    public:
        TextureGraphicPresenter() = default;
        ~TextureGraphicPresenter() override;

        bool init(GraphicDevice& _graphicDevice, const gpu::Texture2DSize& _size, gpu::GraphicResourceFormat _format);
        void release() override;

        bool prepare() override;
        gpu::Texture2DPointer getTargetTexture() override;
        bool shouldClearTarget() const override;
        gpu::GraphicResourceState getPresentationState() const override;
        const gpu::TextureViewPointer& getTextureView() const;

        void present() override;

    private:
        gpu::Texture2DPointer m_targetTexture = nullptr;
        gpu::TextureViewPointer m_textureView = nullptr;
    };

    EGO_POINTER(TextureGraphicPresenter);
} // namespace ego
