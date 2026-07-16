#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/BindingLayout.h"
#include "EgoGraphicHardware/GraphicObjects/Buffer.h"
#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/Pipeline.h"
#include "EgoGraphicHardware/GraphicObjects/ResourceView.h"
#include "EgoGraphicHardware/GraphicObjects/Sampler.h"
#include "EgoGraphicHardware/GraphicObjects/Shader.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiDrawData.h"
#include "EgoGui/Viewport/GuiViewportTypes.h"

namespace ego
{
    class GraphicDevice;
} // namespace ego

namespace ego::gui
{
    struct GuiRenderTextureBinding final
    {
        GuiTextureID m_id = InvalidGuiTextureID;
        gpu::Texture2DReference m_texture = nullptr;
    };

    struct GuiRenderPacket final
    {
        using TextureBindingCollection = std::vector<GuiRenderTextureBinding>;

        GuiViewportID m_viewportID = InvalidGuiViewportID;
        GuiDrawData m_drawData;
        TextureBindingCollection m_textureBindings;
    };

    struct GuiRenderTarget final
    {
        gpu::Texture2DReference m_texture = nullptr;
        gpu::TextureViewReference m_renderTargetView = nullptr;
        gpu::AttachmentLoadOperation m_loadOperation = gpu::AttachmentLoadOperation::Load;
        FloatVector4 m_clearColor = FloatVector4Zero;
    };

    class GuiRender final : public NonCopyable
    {
    public:
        struct InitData final
        {
            gpu::VertexShaderReference m_vertexShader = nullptr;
            gpu::PixelShaderReference m_pixelShader = nullptr;
        };

        GuiRender() = default;
        ~GuiRender() override;

        bool init(GraphicDevice& _graphicDevice, const InitData& _initData);
        void release();
        void clearResources();
        void removeViewport(GuiViewportID _viewportID);

        // The caller must finish GPU reads before replacing or removing resources for the same viewport.
        bool prepare(GraphicDevice& _graphicDevice, GuiRenderPacket _packet);

        // Records a complete rendering scope and leaves the target in RenderTarget state. Submission and final transitions belong to the caller.
        bool record(GraphicDevice& _graphicDevice, const gpu::GraphicCommandListReference& _commandList, const GuiRenderTarget& _target, GuiViewportID _viewportID);

        bool isInitialized() const;

    private:
        struct PreparedTextureBinding final
        {
            GuiTextureID m_id = InvalidGuiTextureID;
            gpu::Texture2DReference m_texture = nullptr;
            gpu::TextureViewReference m_textureView = nullptr;
        };

        struct ViewportResources final
        {
            GuiDrawData m_drawData;
            std::vector<PreparedTextureBinding> m_textureBindings;
            gpu::BufferReference m_vertexBuffer = nullptr;
            gpu::BufferReference m_indexBuffer = nullptr;
            uint32_t m_vertexBufferSize = 0;
            uint32_t m_indexBufferSize = 0;
        };

        struct PipelineEntry final
        {
            gpu::GraphicResourceFormat m_targetFormat = gpu::GraphicResourceFormat::Undefined;
            uint32_t m_sampleCount = 1;
            gpu::GraphicPipelineReference m_pipeline = nullptr;
        };

        bool initDefaultSampler(GraphicDevice& _graphicDevice);
        bool initBindingLayout(GraphicDevice& _graphicDevice);
        gpu::GraphicPipelineReference getOrCreatePipeline(GraphicDevice& _graphicDevice, gpu::GraphicResourceFormat _targetFormat, uint32_t _sampleCount);

        bool prepareTextureBindings(GraphicDevice& _graphicDevice, const GuiRenderPacket::TextureBindingCollection& _textureBindings, ViewportResources& _resources);
        bool prepareBuffers(GraphicDevice& _graphicDevice, ViewportResources& _resources);
        bool prepareVertexBuffer(GraphicDevice& _graphicDevice, ViewportResources& _resources);
        bool prepareIndexBuffer(GraphicDevice& _graphicDevice, ViewportResources& _resources);

        void transitionTextureBindings(const gpu::GraphicCommandListReference& _commandList, const ViewportResources& _resources) const;
        void setupTargetViewport(const gpu::GraphicCommandListReference& _commandList, const gpu::Texture2DSize& _targetSize) const;
        bool renderDrawData(
            const gpu::GraphicCommandListReference& _commandList,
            const gpu::GraphicPipelineReference& _pipeline,
            const gpu::Texture2DSize& _targetSize,
            const ViewportResources& _resources) const;
        uint32_t findTextureBindlessIndex(const ViewportResources& _resources, GuiTextureID _textureID) const;

        static gpu::InputLayoutDesc CreateGuiInputLayout();

        using ViewportResourceMap = std::unordered_map<GuiViewportID, ViewportResources>;

        gpu::VertexShaderReference m_vertexShader = nullptr;
        gpu::PixelShaderReference m_pixelShader = nullptr;
        gpu::SamplerReference m_defaultSampler = nullptr;
        gpu::BindingLayoutReference m_bindingLayout = nullptr;
        std::vector<PipelineEntry> m_pipelines;
        ViewportResourceMap m_viewportResources;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiRender);
} // namespace ego::gui
