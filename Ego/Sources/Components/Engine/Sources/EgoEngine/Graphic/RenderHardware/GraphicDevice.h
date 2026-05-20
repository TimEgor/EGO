#pragma once

#include "GraphicObjects/BindingLayout.h"
#include "GraphicObjects/Buffer.h"
#include "GraphicObjects/CommandList.h"
#include "GraphicObjects/CommandQueue.h"
#include "GraphicObjects/Fence.h"
#include "GraphicObjects/Pipeline.h"
#include "GraphicObjects/ResourceView.h"
#include "GraphicObjects/Sampler.h"
#include "GraphicObjects/Shader.h"
#include "GraphicObjects/SwapChain.h"
#include "GraphicObjects/Texture.h"

namespace ego
{
    class Window;
}

namespace ego::gpu
{
    struct GraphicDeviceInitParams final
    {
        bool m_debugEnable = false;
        bool m_gpuValidation = false;
    };

    struct GraphicDeviceCapabilities final
    {
        uint32_t m_maxColorAttachments = 0;
        bool m_supportsPresentation = true;
        bool m_supportsIndependentComputeQueue = false;
        bool m_supportsMeshShaders = false;
        bool m_supportsRayTracing = false;
        bool m_supportsSamplerAnisotropy = false;
        bool m_supportsBindlessResources = false;
        uint32_t m_bindlessResourceDescriptorCount = 0;
        uint32_t m_bindlessSamplerDescriptorCount = 0;
    };

    class GraphicDevice : public GraphicObject
    {
    public:
        GraphicDevice() = default;

        virtual bool init(const GraphicDeviceInitParams& _params) = 0;
        virtual void release() = 0;

        virtual CommandQueuePointer createCommandQueue(const CommandQueueDesc& _desc) = 0;
        virtual GraphicCommandListPointer createGraphicCommandList() = 0;
        virtual ComputeCommandListPointer createComputeCommandList() = 0;
        virtual CopyCommandListPointer createCopyCommandList() = 0;

        virtual BufferPointer createBuffer(
            const BufferDesc& _desc,
            const InitialGraphicResourceData& _initialData = InitialGraphicResourceData()
        ) = 0;
        virtual Texture2DPointer createTexture2D(
            const Texture2DDesc& _desc,
            const InitialGraphicResourceData& _initialData = InitialGraphicResourceData()
        ) = 0;

        virtual VertexShaderPointer createVertexShader(const ShaderCodePointer& _code) = 0;
        virtual PixelShaderPointer createPixelShader(const ShaderCodePointer& _code) = 0;
        virtual ComputeShaderPointer createComputeShader(const ShaderCodePointer& _code) = 0;
        virtual BindingLayoutPointer createBindingLayout(const BindingLayoutDesc& _desc) = 0;
        virtual SamplerPointer createSampler(const SamplerDesc& _desc) = 0;

        virtual BufferViewPointer createBufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc) = 0;
        virtual TextureViewPointer createTextureView(const TexturePointer& _texture, const TextureViewDesc& _desc) = 0;

        virtual GraphicPipelinePointer createGraphicPipeline(const GraphicPipelineDesc& _desc) = 0;
        virtual ComputePipelinePointer createComputePipeline(const ComputePipelineDesc& _desc) = 0;

        virtual FencePointer createFence(Fence::FenceValue _initialValue = 0) = 0;

        virtual SwapChainPointer createSwapChain(const SwapChainDesc& _swapChainDesc, const Window& _window) = 0;

        virtual const GraphicDeviceCapabilities& getCapabilities() const = 0;

        virtual void waitIdle() = 0;
    };

    EGO_POINTER(GraphicDevice);
}
