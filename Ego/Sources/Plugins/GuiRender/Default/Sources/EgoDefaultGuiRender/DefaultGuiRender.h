#pragma once

#include <cstdint>
#include <vector>

#include "EgoGraphicHardware/GraphicObjects/BindingLayout.h"
#include "EgoGraphicHardware/GraphicObjects/Buffer.h"
#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/Fence.h"
#include "EgoGraphicHardware/GraphicObjects/Pipeline.h"
#include "EgoGraphicHardware/GraphicObjects/Sampler.h"
#include "EgoGraphicHardware/GraphicObjects/Shader.h"

#include "EgoGui/Rendering/GuiRender.h"

#include "DefaultGuiRenderFileSystems.h"

namespace ego
{
    class ResourceController;
} // namespace ego

namespace ego::gui::default_gui_render
{
    class DefaultGuiRender final : public GuiRender
    {
    public:
        DefaultGuiRender() = default;
        ~DefaultGuiRender() override;

        bool init() override;
        void release() override;
        void clearResources() override;

        bool prepare(GuiRenderData&& _renderData) override;
        bool render(const TargetCollection& _targets) override;

    private:
        struct ViewportResources final
        {
            DrawData m_drawData;
            gpu::BufferReference m_vertexBuffer = nullptr;
            gpu::BufferReference m_indexBuffer = nullptr;
        };

        bool initShaders(ResourceController& _resourceController);
        bool initDefaultSampler();
        bool initBindingLayout();
        bool initPipeline();

        bool prepareBuffers(ViewportResources& _resources);
        bool prepareBuffer(const gpu::InitialGraphicResourceData& _initialData, uint32_t _stride, gpu::GraphicResourceUsage _usage, gpu::BufferReference& _buffer);

        bool renderViewport(const gpu::TextureViewReference& _targetView, const gpu::Texture2DReference& _targetTexture, const ViewportResources& _resources);
        bool renderDrawData(const gpu::Texture2DSize& _targetSize, const ViewportResources& _resources) const;
        gpu::Texture2DReference resolveTargetTexture(const gpu::TextureViewReference& _targetView) const;
        void transitionTextureViews() const;

        void waitFrame();
        void signalFrameFence();

        static gpu::InputLayoutDesc CreateGuiInputLayout();

        DefaultGuiRenderFileSystems m_fileSystems;

        gpu::CommandQueueReference m_commandQueue = nullptr;
        gpu::GraphicCommandListReference m_commandList = nullptr;
        gpu::FenceReference m_frameFence = nullptr;
        gpu::Fence::FenceValue m_frameFenceValue = 0;

        gpu::VertexShaderReference m_vertexShader = nullptr;
        gpu::PixelShaderReference m_pixelShader = nullptr;
        gpu::SamplerReference m_defaultSampler = nullptr;
        gpu::BindingLayoutReference m_bindingLayout = nullptr;
        gpu::GraphicPipelineReference m_pipeline = nullptr;

        GuiRenderData::TextureViewCollection m_textureViews;
        std::vector<ViewportResources> m_viewportResources;
        TargetCollection m_targetViews;

        bool m_isInitialized = false;
    };

    EGO_POINTER(DefaultGuiRender);
} // namespace ego::gui::default_gui_render
