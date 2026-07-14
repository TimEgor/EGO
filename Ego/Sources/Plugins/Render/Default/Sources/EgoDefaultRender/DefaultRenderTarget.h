#pragma once

#include "EgoGraphicHardware/GraphicDevice.h"

#include "EgoEngine/Graphic/Render/Render.h"
#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    class DefaultRenderTarget final
    {
    public:
        DefaultRenderTarget() = default;

        bool prepare(GraphicDevice& _graphicDevice, const gpu::Texture2DSize& _resolution, gpu::GraphicResourceFormat _format);
        void release();
        void transition(const RenderGraphicCommandList& _commandList, gpu::GraphicResourceState _nextState);

        bool isReady() const;
        const RenderTexture2D& getTexture() const;
        const RenderTextureView& getRenderTargetView() const;
        const RenderTextureView& getUnorderedAccessView() const;
        const gpu::Texture2DSize& getResolution() const;

    private:
        RenderTexture2D m_texture = nullptr;
        RenderTextureView m_renderTargetView = nullptr;
        RenderTextureView m_unorderedAccessView = nullptr;
        gpu::Texture2DSize m_resolution = DefaultRenderResolution;
    };
} // namespace ego::render
