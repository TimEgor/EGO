#include "MaterialResource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

namespace
{
    constexpr uint32_t DefaultVertexStride = sizeof(float) * 7;

    const char* ReadText(const ego::XmlNode& _node, const char* _name)
    {
        const ego::XmlNode child = _node.getChild(_name);
        return child ? child.getValue().get<const char*>() : nullptr;
    }

    ego::gpu::InputLayoutDesc CreateDefaultInputLayout()
    {
        ego::gpu::InputLayoutDesc inputLayout;

        ego::gpu::InputLayoutBindingDesc bindingDesc;
        bindingDesc.m_slot = 0;
        bindingDesc.m_stride = DefaultVertexStride;
        bindingDesc.m_type = ego::gpu::InputLayoutBindingType::VertexBinding;
        inputLayout.m_bindings.push_back(bindingDesc);

        ego::gpu::InputLayoutElementDesc positionDesc;
        positionDesc.m_semanticName = "POSITION";
        positionDesc.m_location = 0;
        positionDesc.m_index = 0;
        positionDesc.m_slot = 0;
        positionDesc.m_offset = 0;
        positionDesc.m_componentsCount = 3;
        positionDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
        inputLayout.m_elements.push_back(positionDesc);

        ego::gpu::InputLayoutElementDesc colorDesc;
        colorDesc.m_semanticName = "COLOR";
        colorDesc.m_location = 1;
        colorDesc.m_index = 0;
        colorDesc.m_slot = 0;
        colorDesc.m_offset = sizeof(float) * 3;
        colorDesc.m_componentsCount = 4;
        colorDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
        inputLayout.m_elements.push_back(colorDesc);

        return inputLayout;
    }
}

const ego::gpu::GraphicPipelinePointer& ego::MaterialResource::getPipeline() const
{
    return m_pipeline;
}

void ego::MaterialResource::setPipeline(const gpu::GraphicPipelinePointer& _pipeline)
{
    m_pipeline = _pipeline;
}

const std::vector<ego::gpu::ResourceViewPointer>& ego::MaterialResource::getResourceViews() const
{
    return m_resourceViews;
}

void ego::MaterialResource::addResourceView(const gpu::ResourceViewPointer& _resourceView)
{
    if (!_resourceView)
    {
        return;
    }

    m_resourceViews.push_back(_resourceView);
}

void ego::MaterialResource::clearResourceViews()
{
    m_resourceViews.clear();
}

const std::vector<ego::gpu::SamplerPointer>& ego::MaterialResource::getSamplers() const
{
    return m_samplers;
}

void ego::MaterialResource::addSampler(const gpu::SamplerPointer& _sampler)
{
    if (!_sampler)
    {
        return;
    }

    m_samplers.push_back(_sampler);
}

void ego::MaterialResource::clearSamplers()
{
    m_samplers.clear();
}

bool ego::MaterialResource::onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext)
{
    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(!_content.empty() && document.loadFromBuffer(_content.data(), _content.size()));

    const XmlNode materialNode = document.getRootNode().getChild("Material");
    EGO_CHECK_RETURN_FALSE(materialNode);

    const char* vertexShaderPath = ReadText(materialNode, "VertexShader");
    const char* pixelShaderPath = ReadText(materialNode, "PixelShader");
    EGO_CHECK_RETURN_FALSE(vertexShaderPath && vertexShaderPath[0]);
    EGO_CHECK_RETURN_FALSE(pixelShaderPath && pixelShaderPath[0]);

    gpu::VertexShaderResourcePointer vertexShaderResource =
        _loadingContext.load<gpu::VertexShaderResource>(vertexShaderPath);
    gpu::PixelShaderResourcePointer pixelShaderResource =
        _loadingContext.load<gpu::PixelShaderResource>(pixelShaderPath);

    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    gpu::GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();

    gpu::BindingLayoutDesc bindingLayoutDesc;
    gpu::BindingLayoutPointer bindingLayout = graphicDevice.createBindingLayout(bindingLayoutDesc);
    EGO_CHECK_RETURN_FALSE(bindingLayout);

    gpu::GraphicPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = bindingLayout;
    pipelineDesc.m_vertexShader = vertexShaderResource->getVertexShader();
    pipelineDesc.m_pixelShader = pixelShaderResource->getPixelShader();
    pipelineDesc.m_inputLayoutDesc = CreateDefaultInputLayout();
    pipelineDesc.m_topology = gpu::PrimitiveTopology::TriangleList;
    pipelineDesc.m_rasterizationStateDesc.m_cullMode = gpu::RasterizationCullMode::None;
    pipelineDesc.m_depthFormat = gpu::GraphicResourceFormat::Undefined;
    pipelineDesc.m_colorFormats.push_back(gpu::GraphicResourceFormat::B8G8R8A8UNorm);

    gpu::GraphicPipelinePointer pipeline = graphicDevice.createGraphicPipeline(pipelineDesc);
    EGO_CHECK_RETURN_FALSE(pipeline);

    setPipeline(pipeline);
    return true;
}

void ego::MaterialResource::onUnload()
{
    m_pipeline = nullptr;
    m_resourceViews.clear();
    m_samplers.clear();
}
