#pragma once

#include <cstdint>

#include "EgoGui/GuiDrawData.h"

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

#include "EgoDefaultRender/RenderGraph/RenderPass.h"

namespace ego::render
{
    class GuiRenderPass final : public RenderPass
    {
    public:
        GuiRenderPass() = default;

        bool init(RenderPassInitContext& _context) override;
        void release() override;
        void clearResources() override;
        void declare(RenderPassBuilder& _builder) override;
        bool prepare(RenderPassPrepareContext& _context) override;
        void execute(RenderPassExecuteContext& _context) override;

    private:
        bool loadShaders();
        bool initFontSampler(GraphicDevice& _graphicDevice);
        bool initBindingLayout(GraphicDevice& _graphicDevice);
        bool initPipeline(GraphicDevice& _graphicDevice, gpu::GraphicResourceFormat _targetFormat);
        bool prepareFontTextureView(
            GraphicDevice& _graphicDevice,
            const gpu::Texture2DReference& _fontTexture);
        bool prepareBuffers(GraphicDevice& _graphicDevice);
        bool prepareVertexBuffer(GraphicDevice& _graphicDevice);
        bool prepareIndexBuffer(GraphicDevice& _graphicDevice);
        void setupTargetViewport(RenderPassExecuteContext& _context) const;
        void renderDrawData(RenderPassExecuteContext& _context) const;

        static gpu::InputLayoutDesc CreateGuiInputLayout();

        RenderBindingLayout m_bindingLayout = nullptr;
        RenderSampler m_fontSampler = nullptr;
        RenderTextureView m_fontTextureView = nullptr;
        RenderVertexShader m_vertexShader = nullptr;
        RenderPixelShader m_pixelShader = nullptr;
        RenderGraphicPipeline m_pipeline = nullptr;
        RenderBuffer m_vertexBuffer = nullptr;
        RenderBuffer m_indexBuffer = nullptr;
        uint32_t m_vertexBufferSize = 0;
        uint32_t m_indexBufferSize = 0;
        gui::GuiDrawData m_drawData;
    };
} // namespace ego::render
