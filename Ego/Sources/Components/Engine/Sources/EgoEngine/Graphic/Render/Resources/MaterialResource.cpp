#include <string>

#include "MaterialResource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Graphic/Render/RenderShaderData.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

namespace
{
    constexpr uint32_t DefaultVertexStride = sizeof(float) * 7;

    std::string ReadText(const ego::XmlNode& _node, const char* _name)
    {
        return _node.getChildValueOr<std::string>(_name, "");
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

const ego::MaterialReference& ego::MaterialResource::getMaterial() const
{
    return m_material;
}

void ego::MaterialResource::setMaterial(const MaterialReference& _material)
{
    m_material = _material;
}

bool ego::MaterialResource::onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext)
{
    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(!_content.empty() && document.loadFromBuffer(_content.data(), _content.size()));

    const XmlNode materialNode = document.getRootNode();
    EGO_CHECK_RETURN_FALSE(materialNode);
    EGO_CHECK_RETURN_FALSE(materialNode.getNameView() == "Material");

    const std::string vertexShaderPath = ReadText(materialNode, "VertexShader");
    const std::string pixelShaderPath = ReadText(materialNode, "PixelShader");
    EGO_CHECK_RETURN_FALSE(!vertexShaderPath.empty());
    EGO_CHECK_RETURN_FALSE(!pixelShaderPath.empty());

    gpu::VertexShaderResourcePointer vertexShaderResource =
        _loadingContext.loadResource<gpu::VertexShaderResource>(vertexShaderPath.c_str());
    gpu::PixelShaderResourcePointer pixelShaderResource =
        _loadingContext.loadResource<gpu::PixelShaderResource>(pixelShaderPath.c_str());

    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();

    gpu::BindingLayoutDesc bindingLayoutDesc;
    gpu::PushConstantRangeDesc renderBindlessRootConstants;
    renderBindlessRootConstants.m_offset = RenderBindlessRootConstantsOffset;
    renderBindlessRootConstants.m_size = RenderBindlessRootConstantsSize;
    renderBindlessRootConstants.m_stageFlag = RenderBindlessRootConstantsStageFlag;
    bindingLayoutDesc.m_pushConstants.push_back(renderBindlessRootConstants);

    gpu::BindingLayoutReference bindingLayout = graphicDevice.createBindingLayout(bindingLayoutDesc);
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

    gpu::GraphicPipelineReference pipeline = graphicDevice.createGraphicPipeline(pipelineDesc);
    EGO_CHECK_RETURN_FALSE(pipeline);

    MaterialReference material = new Material(pipeline);

    setMaterial(material);
    return true;
}

void ego::MaterialResource::onUnload()
{
    m_material = nullptr;
}

ego::MaterialHandle ego::CreateMaterialHandle(const MaterialResourcePointer& _resource)
{
    return MakeHandle<Material>(
        _resource,
        [](const MaterialResourcePointer& _storedResource) -> Material*
        {
            return _storedResource ? _storedResource->getMaterial().getObject() : nullptr;
        }
    );
}
