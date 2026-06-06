#include "MaterialResource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/Render.h"
#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

const ego::render::RenderMaterial& ego::render::MaterialResource::getMaterial() const
{
    return m_material;
}

bool ego::render::MaterialResource::onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext)
{
    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(!_content.empty() && document.loadFromBuffer(_content.data(), _content.size()));

    const XmlNode materialNode = document.getRootNode();
    EGO_CHECK_RETURN_FALSE(materialNode);
    EGO_CHECK_RETURN_FALSE(materialNode.getNameView() == "Material");

    const XmlNode vertexShaderNode = materialNode.getChild("VertexShader");
    const XmlNode pixelShaderNode = materialNode.getChild("PixelShader");
    EGO_CHECK_RETURN_FALSE(vertexShaderNode);
    EGO_CHECK_RETURN_FALSE(pixelShaderNode);

    const FileName vertexShaderPath = FileName(vertexShaderNode.getValue().getRaw());
    const FileName pixelShaderPath = FileName(pixelShaderNode.getValue().getRaw());
    EGO_CHECK_RETURN_FALSE(vertexShaderPath);
    EGO_CHECK_RETURN_FALSE(pixelShaderPath);

    gpu::VertexShaderResourcePointer vertexShaderResource =
        _loadingContext.loadChildResource<gpu::VertexShaderResource>(vertexShaderPath);
    gpu::PixelShaderResourcePointer pixelShaderResource =
        _loadingContext.loadChildResource<gpu::PixelShaderResource>(pixelShaderPath);

    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    const RenderVertexShader vertexShader = CreateVertexShaderHandler(vertexShaderResource);
    const RenderPixelShader pixelShader = CreatePixelShaderHandler(pixelShaderResource);

    const RenderGraphicPipeline pipeline = engine::GetEngine().getRender().createPipeline(vertexShader, pixelShader);
    EGO_CHECK_RETURN_FALSE(pipeline);

    RenderMaterial material = MaterialReference(new Material(pipeline));
    m_material = material;

    return true;
}

void ego::render::MaterialResource::onUnload()
{
    m_material = nullptr;
}

ego::render::RenderMaterial ego::render::CreateMaterialHandler(const MaterialResourcePointer& _resource)
{
    return MakeHandler<MaterialReference>(
        _resource,
        [](const MaterialResourcePointer& _storedResource) -> RenderMaterial
        {
            return _storedResource ? _storedResource->getMaterial() : nullptr;
        }
    );
}
