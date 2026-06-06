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

bool ego::render::MaterialResource::onLoad(
    FileContent&& _content,
    ResourceLoadingContext& _loadingContext
)
{
    m_material = nullptr;

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(
        !_content.empty() && document.loadFromBuffer(_content.data(), _content.size())
    );

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

    if (_loadingContext.isAsyncLoading())
    {
        gpu::VertexShaderResourcePointer vertexShaderResource =
            _loadingContext.loadAsyncDependency<gpu::VertexShaderResource>(vertexShaderPath);
        gpu::PixelShaderResourcePointer pixelShaderResource =
            _loadingContext.loadAsyncDependency<gpu::PixelShaderResource>(pixelShaderPath);

        if (!vertexShaderResource || !pixelShaderResource)
        {
            setLoadingError("Failed to load material shader dependencies.");
            return false;
        }

        setDependenciesLoadedCallback(
            [this, vertexShaderResource, pixelShaderResource]()
            {
                return completeLoading(vertexShaderResource, pixelShaderResource);
            }
        );

        return true;
    }

    gpu::VertexShaderResourcePointer vertexShaderResource =
        _loadingContext.loadDependency<gpu::VertexShaderResource>(vertexShaderPath);
    gpu::PixelShaderResourcePointer pixelShaderResource =
        _loadingContext.loadDependency<gpu::PixelShaderResource>(pixelShaderPath);

    if (!vertexShaderResource || !pixelShaderResource)
    {
        setLoadingError("Failed to load material shader dependencies.");
        return false;
    }

    return completeLoading(vertexShaderResource, pixelShaderResource);
}

bool ego::render::MaterialResource::completeLoading(
    const SharedPointer<gpu::VertexShaderResource>& _vertexShaderResource,
    const SharedPointer<gpu::PixelShaderResource>& _pixelShaderResource
)
{
    if (!_vertexShaderResource || !_vertexShaderResource->isLoaded())
    {
        setLoadingError("Vertex shader resource hasn't been loaded.");
        return false;
    }

    if (!_pixelShaderResource || !_pixelShaderResource->isLoaded())
    {
        setLoadingError("Pixel shader resource hasn't been loaded.");
        return false;
    }

    const RenderVertexShader vertexShader = CreateVertexShaderHandler(_vertexShaderResource);
    const RenderPixelShader pixelShader = CreatePixelShaderHandler(_pixelShaderResource);
    if (!vertexShader || !pixelShader)
    {
        setLoadingError("Failed to create material shader handlers.");
        return false;
    }

    const RenderGraphicPipeline pipeline = engine::GetEngine().getRender().createPipeline(vertexShader, pixelShader);
    if (!pipeline)
    {
        setLoadingError("Failed to create material pipeline.");
        return false;
    }

    RenderMaterial material = MaterialReference(new Material(pipeline));
    if (!material)
    {
        setLoadingError("Failed to create material.");
        return false;
    }

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
