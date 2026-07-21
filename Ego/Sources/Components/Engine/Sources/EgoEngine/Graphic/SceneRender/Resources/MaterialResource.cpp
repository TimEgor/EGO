#include "MaterialResource.h"

#include <string>

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/ResourceLoadingContext.h"

#include "EgoGraphicHardware/Resources/ShaderResource.h"

namespace ego::render
{
    constexpr const char* RasterizationMaterialPassTypeName = "Rasterization";
    constexpr const char* RayTracingMaterialPassTypeName = "RayTracing";
} // namespace ego::render

const ego::render::RenderMaterial& ego::render::MaterialResource::getMaterial() const
{
    return m_material;
}

template <typename TResource>
bool ego::render::MaterialResource::loadRequiredShaderDependency(
    const XmlNode& _passNode,
    const std::string& _passName,
    const char* _childName,
    ResourceLoadingContext& _loadingContext,
    SharedPointer<TResource>& _resource)
{
    FileName shaderPath;
    if (!readRequiredFileName(_passNode, _passName, _childName, shaderPath))
    {
        return false;
    }

    _resource = loadShaderDependency<TResource>(_loadingContext, shaderPath);
    if (!_resource)
    {
        setLoadingError("Failed to load material pass '" + _passName + "' " + _childName + " dependency.");
        return false;
    }

    return true;
}

template <typename TResource>
ego::SharedPointer<TResource> ego::render::MaterialResource::loadShaderDependency(
    ResourceLoadingContext& _loadingContext,
    const FileName& _path)
{
    if (_loadingContext.isAsyncLoading())
    {
        return _loadingContext.loadAsyncDependency<TResource>(_path);
    }

    return _loadingContext.loadDependency<TResource>(_path);
}

template <typename TResource>
bool ego::render::MaterialResource::loadOptionalShaderDependency(
    const XmlNode& _passNode,
    const std::string& _passName,
    const char* _childName,
    ResourceLoadingContext& _loadingContext,
    SharedPointer<TResource>& _resource)
{
    const std::string value = _passNode.getChildValueOr<std::string>(_childName, std::string());
    if (value.empty())
    {
        _resource = nullptr;
        return true;
    }

    const FileName shaderPath(value);
    if (!shaderPath)
    {
        setLoadingError("Material pass '" + _passName + "' has invalid " + _childName + " path.");
        return false;
    }

    _resource = loadShaderDependency<TResource>(_loadingContext, shaderPath);
    if (!_resource)
    {
        setLoadingError("Failed to load material pass '" + _passName + "' " + _childName + " dependency.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext)
{
    m_material = nullptr;
    clearPassData();

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(!_content.empty() && document.loadFromBuffer(_content.data(), _content.size()));

    const XmlNode materialNode = document.getRootNode();
    EGO_CHECK_RETURN_FALSE(materialNode);
    EGO_CHECK_RETURN_FALSE(materialNode.getNameView() == "Material");

    if (!loadPasses(materialNode, _loadingContext))
    {
        return false;
    }

    if (_loadingContext.isAsyncLoading())
    {
        setDependenciesLoadedCallback(
            [this]()
            {
                return buildMaterial();
            });
        return true;
    }

    return buildMaterial();
}

bool ego::render::MaterialResource::loadPasses(const XmlNode& _materialNode, ResourceLoadingContext& _loadingContext)
{
    for (const XmlNode passNode : _materialNode.getChildren("Pass"))
    {
        if (!loadPass(passNode, _loadingContext))
        {
            return false;
        }
    }

    if (m_rasterizationPasses.empty() && m_rayTracingPasses.empty())
    {
        setLoadingError("Material must contain at least one Pass.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::loadPass(const XmlNode& _passNode, ResourceLoadingContext& _loadingContext)
{
    std::string passName;
    if (!readPassName(_passNode, passName))
    {
        return false;
    }

    std::string passType;
    if (!readPassType(_passNode, passName, passType))
    {
        return false;
    }

    if (passType == RasterizationMaterialPassTypeName)
    {
        return loadRasterizationPass(_passNode, passName, _loadingContext);
    }

    if (passType == RayTracingMaterialPassTypeName)
    {
        return loadRayTracingPass(_passNode, passName, _loadingContext);
    }

    setLoadingError("Material pass '" + passName + "' has unsupported type '" + passType + "'.");
    return false;
}

bool ego::render::MaterialResource::loadRasterizationPass(
    const XmlNode& _passNode,
    const std::string& _passName,
    ResourceLoadingContext& _loadingContext)
{
    RasterizationPassData passData;
    passData.m_name = _passName;

    if (!loadRequiredShaderDependency<gpu::VertexShaderResource>(
            _passNode,
            _passName,
            "VertexShader",
            _loadingContext,
            passData.m_vertexShaderResource))
    {
        return false;
    }

    if (!loadRequiredShaderDependency<gpu::PixelShaderResource>(
            _passNode,
            _passName,
            "PixelShader",
            _loadingContext,
            passData.m_pixelShaderResource))
    {
        return false;
    }

    m_rasterizationPasses.push_back(passData);
    return true;
}

bool ego::render::MaterialResource::loadRayTracingPass(
    const XmlNode& _passNode,
    const std::string& _passName,
    ResourceLoadingContext& _loadingContext)
{
    RayTracingPassData passData;
    passData.m_name = _passName;

    if (!loadRequiredShaderDependency<gpu::ClosestHitShaderResource>(
            _passNode,
            _passName,
            "ClosestHitShader",
            _loadingContext,
            passData.m_closestHitShaderResource))
    {
        return false;
    }

    if (!loadOptionalShaderDependency<gpu::AnyHitShaderResource>(
            _passNode,
            _passName,
            "AnyHitShader",
            _loadingContext,
            passData.m_anyHitShaderResource))
    {
        return false;
    }

    if (!loadOptionalShaderDependency<gpu::IntersectionShaderResource>(
            _passNode,
            _passName,
            "IntersectionShader",
            _loadingContext,
            passData.m_intersectionShaderResource))
    {
        return false;
    }

    m_rayTracingPasses.push_back(passData);
    return true;
}

bool ego::render::MaterialResource::buildMaterial()
{
    m_material = nullptr;

    const MaterialTemplateReference materialTemplate(new MaterialTemplate());
    if (!materialTemplate)
    {
        setLoadingError("Failed to create material template.");
        return false;
    }

    for (const RasterizationPassData& passData : m_rasterizationPasses)
    {
        if (!addRasterizationPassToTemplate(passData, materialTemplate))
        {
            return false;
        }
    }

    for (const RayTracingPassData& passData : m_rayTracingPasses)
    {
        if (!addRayTracingPassToTemplate(passData, materialTemplate))
        {
            return false;
        }
    }

    m_material = MaterialReference(new Material(materialTemplate));
    if (!m_material)
    {
        setLoadingError("Failed to create material.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::addRasterizationPassToTemplate(
    const RasterizationPassData& _passData,
    const MaterialTemplateReference& _materialTemplate)
{
    if (!_passData.m_vertexShaderResource || !_passData.m_vertexShaderResource->isLoaded())
    {
        setLoadingError("Material pass '" + _passData.m_name + "' vertex shader resource hasn't been loaded.");
        return false;
    }

    if (!_passData.m_pixelShaderResource || !_passData.m_pixelShaderResource->isLoaded())
    {
        setLoadingError("Material pass '" + _passData.m_name + "' pixel shader resource hasn't been loaded.");
        return false;
    }

    const RenderVertexShader vertexShader = CreateVertexShaderHandler(_passData.m_vertexShaderResource);
    const RenderPixelShader pixelShader = CreatePixelShaderHandler(_passData.m_pixelShaderResource);
    if (!vertexShader || !pixelShader)
    {
        setLoadingError("Failed to create material pass '" + _passData.m_name + "' rasterization shader handlers.");
        return false;
    }

    const RasterizationMaterialRenderPassInfoReference passInfo(new RasterizationMaterialRenderPassInfo(vertexShader, pixelShader));
    if (!passInfo)
    {
        setLoadingError("Failed to create material pass '" + _passData.m_name + "' rasterization info.");
        return false;
    }

    return addPassToTemplate(_passData.m_name, passInfo, _materialTemplate);
}

bool ego::render::MaterialResource::addRayTracingPassToTemplate(
    const RayTracingPassData& _passData,
    const MaterialTemplateReference& _materialTemplate)
{
    if (!_passData.m_closestHitShaderResource || !_passData.m_closestHitShaderResource->isLoaded())
    {
        setLoadingError("Material pass '" + _passData.m_name + "' closest hit shader resource hasn't been loaded.");
        return false;
    }

    if (_passData.m_anyHitShaderResource && !_passData.m_anyHitShaderResource->isLoaded())
    {
        setLoadingError("Material pass '" + _passData.m_name + "' any hit shader resource hasn't been loaded.");
        return false;
    }

    if (_passData.m_intersectionShaderResource && !_passData.m_intersectionShaderResource->isLoaded())
    {
        setLoadingError("Material pass '" + _passData.m_name + "' intersection shader resource hasn't been loaded.");
        return false;
    }

    RayTracingMaterialHitGroup hitGroup;
    hitGroup.m_closestHitShader = CreateClosestHitShaderHandler(_passData.m_closestHitShaderResource);
    if (!hitGroup.m_closestHitShader)
    {
        setLoadingError("Failed to create material pass '" + _passData.m_name + "' closest hit shader handler.");
        return false;
    }

    if (_passData.m_anyHitShaderResource)
    {
        hitGroup.m_anyHitShader = CreateAnyHitShaderHandler(_passData.m_anyHitShaderResource);
        if (!hitGroup.m_anyHitShader)
        {
            setLoadingError("Failed to create material pass '" + _passData.m_name + "' any hit shader handler.");
            return false;
        }
    }

    if (_passData.m_intersectionShaderResource)
    {
        hitGroup.m_intersectionShader = CreateIntersectionShaderHandler(_passData.m_intersectionShaderResource);
        if (!hitGroup.m_intersectionShader)
        {
            setLoadingError("Failed to create material pass '" + _passData.m_name + "' intersection shader handler.");
            return false;
        }
    }

    const RayTracingMaterialRenderPassInfoReference passInfo(new RayTracingMaterialRenderPassInfo(hitGroup));
    if (!passInfo)
    {
        setLoadingError("Failed to create material pass '" + _passData.m_name + "' ray tracing info.");
        return false;
    }

    return addPassToTemplate(_passData.m_name, passInfo, _materialTemplate);
}

bool ego::render::MaterialResource::addPassToTemplate(
    const std::string& _passName,
    const MaterialRenderPassInfoReference& _passInfo,
    const MaterialTemplateReference& _materialTemplate)
{
    if (_materialTemplate->hasRenderPassInfo(_passName))
    {
        setLoadingError("Material contains duplicate pass id for '" + _passName + "'.");
        return false;
    }

    if (!_materialTemplate->setRenderPassInfo(_passName, _passInfo))
    {
        setLoadingError("Failed to add material pass '" + _passName + "' to material template.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::readPassName(const XmlNode& _passNode, std::string& _name)
{
    const char* rawName = _passNode.getAttributeValue("name");
    if (rawName == nullptr || rawName[0] == '\0')
    {
        rawName = _passNode.getAttributeValue("Name");
    }

    _name = rawName != nullptr ? rawName : std::string();
    if (_name.empty())
    {
        setLoadingError("Material pass has no name.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::readPassType(const XmlNode& _passNode, const std::string& _passName, std::string& _type)
{
    const char* rawType = _passNode.getAttributeValue("type");
    if (rawType == nullptr || rawType[0] == '\0')
    {
        rawType = _passNode.getAttributeValue("Type");
    }

    _type = rawType != nullptr ? rawType : std::string();
    if (_type.empty())
    {
        setLoadingError("Material pass '" + _passName + "' has no type.");
        return false;
    }

    return true;
}

bool ego::render::MaterialResource::readRequiredFileName(
    const XmlNode& _node,
    const std::string& _passName,
    const char* _childName,
    FileName& _fileName)
{
    const std::string value = _node.getChildValueOr<std::string>(_childName, std::string());
    if (value.empty())
    {
        setLoadingError("Material pass '" + _passName + "' has no " + _childName + ".");
        return false;
    }

    _fileName = FileName(value);
    if (!_fileName)
    {
        setLoadingError("Material pass '" + _passName + "' has invalid " + _childName + " path.");
        return false;
    }

    return true;
}

void ego::render::MaterialResource::onUnload()
{
    m_material = nullptr;
    clearPassData();
}

void ego::render::MaterialResource::clearPassData()
{
    m_rasterizationPasses.clear();
    m_rayTracingPasses.clear();
}

ego::render::RenderMaterial ego::render::CreateMaterialHandler(const MaterialResourcePointer& _resource)
{
    return MakeHandler<MaterialReference>(
        _resource,
        [](const MaterialResourcePointer& _storedResource) -> RenderMaterial
        {
            return _storedResource ? _storedResource->getMaterial() : nullptr;
        });
}
