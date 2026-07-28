#include "RayTracingRenderPass.h"

#include <algorithm>
#include <functional>
#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/RTTI/RTTI.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/GeneralResources/XmlResource.h"
#include "EgoResource/ResourceSubsystem.h"
#include "EgoResource/ResourceController.h"

#include "EgoGraphicHardware/Resources/ShaderResource.h"

#include "EgoEngine/Graphic/SceneRender/RenderResourceObject.h"

#include "EgoDefaultRender/DefaultRenderBindingLayout.h"
#include "EgoDefaultRender/DefaultRenderConstants.h"
#include "EgoDefaultRender/PipelineState/RenderPipelineStateCache.h"
#include "EgoDefaultRender/RenderShaderData.h"

namespace ego::render
{
    constexpr auto RayTracingRenderPassConfigPath = "Configs/RayTracingRenderPass.xml";
    constexpr auto RayTracingMaterialPassName = "RayTracing";

    struct RayTracingRenderPassShaderConfig final
    {
        FileName m_rayGenerationShaderPath;
        FileName m_missShaderPath;
    };

    static bool ReadRayTracingRenderPassRequiredFileName(const XmlNode& _node, const char* _childName, FileName& _fileName)
    {
        const std::string value = _node.getChildValueOr<std::string>(_childName, std::string());
        EGO_CHECK_RETURN_FALSE(!value.empty());

        _fileName = FileName(value);
        return static_cast<bool>(_fileName);
    }

    static bool ParseRayTracingRenderPassShaderConfig(const XmlNode& _configNode, RayTracingRenderPassShaderConfig& _config)
    {
        EGO_CHECK_RETURN_FALSE(_configNode && _configNode.getNameView() == "RayTracingRenderPass");

        return ReadRayTracingRenderPassRequiredFileName(_configNode, "RayGenerationShader", _config.m_rayGenerationShaderPath) &&
               ReadRayTracingRenderPassRequiredFileName(_configNode, "MissShader", _config.m_missShaderPath);
    }

    static bool LoadRayTracingRenderPassShaderConfig(RayTracingRenderPassShaderConfig& _config)
    {
        ResourceController& resourceController = GetResourceSubsystem().getResourceController();
        const XmlResourcePointer configResource = resourceController.load<XmlResource>(RayTracingRenderPassConfigPath);
        return configResource && configResource->isLoaded() && ParseRayTracingRenderPassShaderConfig(configResource->getRootNode(), _config);
    }

    static RayTracingMaterialRenderPassInfoPointer GetRayTracingMaterialInfo(const DefaultRenderItem& _item)
    {
        if (!_item.m_material)
        {
            return nullptr;
        }

        const MaterialTemplatePointer& materialTemplate = _item.m_material->getMaterialTemplate();
        if (!materialTemplate)
        {
            return nullptr;
        }

        const MaterialRenderPassInfoPointer passInfo = materialTemplate->getRenderPassInfo(RayTracingMaterialPassName);
        if (!passInfo || !rtti::IsObjectBasedOn<RayTracingMaterialRenderPassInfo>(*passInfo))
        {
            return nullptr;
        }

        return RayTracingMaterialRenderPassInfoPointer(passInfo.getObjectCast<RayTracingMaterialRenderPassInfo>());
    }

} // namespace ego::render

bool ego::render::RayTracingRenderPass::RayTracingHitGroupKey::operator==(const RayTracingHitGroupKey& _other) const
{
    return m_type == _other.m_type && m_closestHitShader == _other.m_closestHitShader && m_anyHitShader == _other.m_anyHitShader &&
           m_intersectionShader == _other.m_intersectionShader;
}

bool ego::render::RayTracingRenderPass::RayTracingHitGroupKey::operator!=(const RayTracingHitGroupKey& _other) const
{
    return !(*this == _other);
}

size_t ego::render::RayTracingRenderPass::RayTracingHitGroupKeyHash::operator()(const RayTracingHitGroupKey& _key) const
{
    size_t hash = std::hash<int>()(static_cast<int>(_key.m_type));
    hash ^= std::hash<const void*>()(_key.m_closestHitShader) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<const void*>()(_key.m_anyHitShader) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<const void*>()(_key.m_intersectionShader) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

bool ego::render::RayTracingRenderPass::init(RenderPassInitContext& _context)
{
    EGO_CHECK_RETURN_FALSE(loadShaders());

    m_bindingLayout = CreateDefaultRenderBindlessBindingLayout(_context.m_graphicDevice);
    EGO_CHECK_RETURN_FALSE(m_bindingLayout);

    return true;
}

void ego::render::RayTracingRenderPass::release()
{
    clearResources();

    m_bindingLayout = nullptr;
    m_rayGenerationShader = nullptr;
    m_missShader = nullptr;
}

void ego::render::RayTracingRenderPass::clearResources()
{
    m_sceneAccelerationStructureView = nullptr;
    m_sceneAccelerationStructure = nullptr;
    m_geometryCache.clear();
    clearHitGroupTable();
}

void ego::render::RayTracingRenderPass::declare(RenderPassBuilder& _builder)
{
    _builder.writeTexture("DefaultRenderTarget", gpu::GraphicResourceState::UnorderedAccess, gpu::GraphicResourceViewType::UnorderedAccess);
    _builder.readBuffer("CameraShaderData");
    _builder.readAccelerationStructure("SceneAccelerationStructure");
}

bool ego::render::RayTracingRenderPass::prepare(RenderPassPrepareContext& _context)
{
    m_sceneAccelerationStructureView = nullptr;
    m_sceneAccelerationStructure = nullptr;
    clearHitGroupTable();
    m_geometryCache.beginFrame();

    const DefaultRenderScene::ItemCollection& renderItems = _context.m_scene.getItems();
    if (renderItems.empty())
    {
        m_geometryCache.removeUnused();
        return true;
    }

    if (!buildHitGroupTable(renderItems))
    {
        m_geometryCache.removeUnused();
        return true;
    }

    gpu::InstanceAccelerationStructureBuildDesc sceneDesc;
    sceneDesc.m_instances.reserve(renderItems.size());

    for (const DefaultRenderItem& item : renderItems)
    {
        const RayTracingMaterialRenderPassInfoPointer materialInfo = GetRayTracingMaterialInfo(item);
        const RayTracingMaterialHitGroup* hitGroup = materialInfo ? &materialInfo->getHitGroup() : nullptr;
        if (!item.m_mesh || !hitGroup)
        {
            continue;
        }

        uint32_t hitGroupIndex = 0;
        if (!findHitGroupIndex(*hitGroup, hitGroupIndex))
        {
            continue;
        }

        RenderGeometryAccelerationStructure geometryAccelerationStructure = m_geometryCache.requestGeometry(item.m_mesh, _context.m_graphicDevice);
        if (!geometryAccelerationStructure || !geometryAccelerationStructure.getObject()->isGpuReady())
        {
            continue;
        }

        const uint32_t instanceIndex = static_cast<uint32_t>(sceneDesc.m_instances.size());
        gpu::InstanceGeometryAccelerationStructureBuildDesc instanceDesc;
        instanceDesc.m_geometry = geometryAccelerationStructure.getObject();
        instanceDesc.m_transform = item.m_globalTransform.m_matrix.getFloatMatrix4x4();
        instanceDesc.m_instanceId = instanceIndex;
        instanceDesc.m_instanceMask = 0xff;
        instanceDesc.m_hitGroupIndex = hitGroupIndex;

        sceneDesc.m_instances.push_back(instanceDesc);
    }

    m_geometryCache.removeUnused();

    if (sceneDesc.m_instances.empty())
    {
        return true;
    }

    const gpu::GpuOperationOptions buildOptions{gpu::GpuCompletionMode::WaitForCompletion};
    const gpu::GpuInstanceAccelerationStructureTicket sceneAccelerationStructure = _context.m_graphicDevice.buildInstanceAccelerationStructure(sceneDesc, buildOptions);
    m_sceneAccelerationStructure = sceneAccelerationStructure.m_resource;
    EGO_CHECK_RETURN_FALSE(m_sceneAccelerationStructure);

    m_sceneAccelerationStructureView = _context.m_graphicDevice.createAccelerationStructureView(m_sceneAccelerationStructure.getObject());
    return static_cast<bool>(m_sceneAccelerationStructureView);
}

void ego::render::RayTracingRenderPass::execute(RenderPassExecuteContext& _context)
{
    if (!_context.m_commandList || m_hitGroupTable.empty() || !_context.m_shaderData.getCameraShaderDataView() || !m_sceneAccelerationStructureView ||
        !_context.m_renderTarget.getUnorderedAccessView())
    {
        return;
    }

    RayTracingBindlessRootConstants rootConstants;
    rootConstants.m_cameraDataIndex = _context.m_shaderData.getCameraShaderDataView()->getBindlessIndex();
    rootConstants.m_sceneDataIndex = m_sceneAccelerationStructureView->getBindlessIndex();
    rootConstants.m_outputDataIndex = _context.m_renderTarget.getUnorderedAccessView()->getBindlessIndex();

    if (rootConstants.m_cameraDataIndex == gpu::InvalidBindlessIndex || rootConstants.m_sceneDataIndex == gpu::InvalidBindlessIndex ||
        rootConstants.m_outputDataIndex == gpu::InvalidBindlessIndex)
    {
        return;
    }

    _context.m_renderTarget.transition(_context.m_commandList, gpu::GraphicResourceState::UnorderedAccess);
    const RenderRayTracingPipeline pipeline = getOrCreatePipeline(_context.m_graphicDevice, _context.m_pipelineStateCache);
    if (!pipeline)
    {
        return;
    }

    _context.m_commandList->setPipeline(pipeline.getObject());
    _context.m_commandList->pushConstants(RenderBindlessRootConstantsStageFlag, RenderBindlessRootConstantsOffset, sizeof(rootConstants), &rootConstants);

    const gpu::Texture2DSize& resolution = _context.m_renderTarget.getResolution();
    gpu::DispatchRaysDesc dispatchDesc;
    dispatchDesc.m_width = resolution.m_x;
    dispatchDesc.m_height = resolution.m_y;
    dispatchDesc.m_depth = 1;
    _context.m_commandList->dispatchRays(dispatchDesc);
}

bool ego::render::RayTracingRenderPass::loadShaders()
{
    RayTracingRenderPassShaderConfig config;
    EGO_CHECK_RETURN_FALSE(LoadRayTracingRenderPassShaderConfig(config));

    ResourceController& resourceController = GetResourceSubsystem().getResourceController();

    const gpu::RayGenerationShaderResourcePointer rayGenerationShaderResource = resourceController.load<gpu::RayGenerationShaderResource>(config.m_rayGenerationShaderPath);
    const gpu::MissShaderResourcePointer missShaderResource = resourceController.load<gpu::MissShaderResource>(config.m_missShaderPath);
    EGO_CHECK_RETURN_FALSE(rayGenerationShaderResource && rayGenerationShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(missShaderResource && missShaderResource->isLoaded());

    const RenderRayGenerationShader rayGenerationShader = CreateRayGenerationShaderHandler(rayGenerationShaderResource);
    const RenderMissShader missShader = CreateMissShaderHandler(missShaderResource);
    EGO_CHECK_RETURN_FALSE(rayGenerationShader && missShader);

    m_rayGenerationShader = rayGenerationShader;
    m_missShader = missShader;
    return true;
}

void ego::render::RayTracingRenderPass::clearHitGroupTable()
{
    m_hitGroupTable.clear();
    m_hitGroupIndices.clear();
}

bool ego::render::RayTracingRenderPass::buildHitGroupTable(const DefaultRenderScene::ItemCollection& _renderItems)
{
    clearHitGroupTable();

    for (const DefaultRenderItem& item : _renderItems)
    {
        const RayTracingMaterialRenderPassInfoPointer materialInfo = GetRayTracingMaterialInfo(item);
        const RayTracingMaterialHitGroup* hitGroup = materialInfo ? &materialInfo->getHitGroup() : nullptr;
        if (!item.m_mesh || !hitGroup)
        {
            continue;
        }

        const RayTracingHitGroupKey key = MakeHitGroupKey(*hitGroup);
        if (!key.m_closestHitShader && !key.m_anyHitShader && !key.m_intersectionShader)
        {
            continue;
        }

        if (m_hitGroupIndices.find(key) != m_hitGroupIndices.end())
        {
            continue;
        }

        RayTracingHitGroupEntry entry;
        entry.m_key = key;
        entry.m_hitGroup = *hitGroup;
        m_hitGroupTable.push_back(entry);
        m_hitGroupIndices.emplace(key, 0);
    }

    if (m_hitGroupTable.empty())
    {
        return false;
    }

    std::sort(
        m_hitGroupTable.begin(),
        m_hitGroupTable.end(),
        [](const RayTracingHitGroupEntry& _left, const RayTracingHitGroupEntry& _right)
        {
            const std::less<const void*> pointerLess;
            if (_left.m_key.m_type != _right.m_key.m_type)
            {
                return _left.m_key.m_type < _right.m_key.m_type;
            }

            if (_left.m_key.m_closestHitShader != _right.m_key.m_closestHitShader)
            {
                return pointerLess(_left.m_key.m_closestHitShader, _right.m_key.m_closestHitShader);
            }

            if (_left.m_key.m_anyHitShader != _right.m_key.m_anyHitShader)
            {
                return pointerLess(_left.m_key.m_anyHitShader, _right.m_key.m_anyHitShader);
            }

            return pointerLess(_left.m_key.m_intersectionShader, _right.m_key.m_intersectionShader);
        });

    m_hitGroupIndices.clear();
    for (size_t shaderIndex = 0; shaderIndex < m_hitGroupTable.size(); ++shaderIndex)
    {
        m_hitGroupIndices.emplace(m_hitGroupTable[shaderIndex].m_key, static_cast<uint32_t>(shaderIndex));
    }

    return true;
}

bool ego::render::RayTracingRenderPass::findHitGroupIndex(const RayTracingMaterialHitGroup& _hitGroup, uint32_t& _index) const
{
    const RayTracingHitGroupKey key = MakeHitGroupKey(_hitGroup);
    if (!key.m_closestHitShader && !key.m_anyHitShader && !key.m_intersectionShader)
    {
        return false;
    }

    const std::unordered_map<RayTracingHitGroupKey, uint32_t, RayTracingHitGroupKeyHash>::const_iterator foundIt = m_hitGroupIndices.find(key);
    if (foundIt == m_hitGroupIndices.end())
    {
        return false;
    }

    _index = foundIt->second;
    return true;
}

ego::render::RenderRayTracingPipeline ego::render::RayTracingRenderPass::getOrCreatePipeline(GraphicDevice& _graphicDevice, RenderPipelineStateCache& _pipelineStateCache) const
{
    if (m_hitGroupTable.empty() || !m_rayGenerationShader || !m_missShader || !m_bindingLayout)
    {
        return nullptr;
    }

    gpu::RayTracingPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = m_bindingLayout.getObject();
    pipelineDesc.m_rayGenerationShader = m_rayGenerationShader.getObject();
    pipelineDesc.m_missShader = m_missShader.getObject();
    pipelineDesc.m_hitGroups.reserve(m_hitGroupTable.size());
    for (const RayTracingHitGroupEntry& hitGroup : m_hitGroupTable)
    {
        pipelineDesc.m_hitGroups.push_back(MakeHitGroupDesc(hitGroup.m_hitGroup));
    }
    pipelineDesc.m_maxPayloadSize = DefaultRenderRayTracingPayloadSize;
    pipelineDesc.m_maxAttributeSize = DefaultRenderRayTracingAttributeSize;
    pipelineDesc.m_maxRecursionDepth = 1;

    return _pipelineStateCache.getOrCreateRayTracingPipeline(_graphicDevice, pipelineDesc);
}

ego::render::RayTracingRenderPass::RayTracingHitGroupKey ego::render::RayTracingRenderPass::MakeHitGroupKey(const RayTracingMaterialHitGroup& _hitGroup)
{
    const gpu::ClosestHitShaderPointer closestHitShader = _hitGroup.m_closestHitShader ? _hitGroup.m_closestHitShader.getObject() : nullptr;
    const gpu::AnyHitShaderPointer anyHitShader = _hitGroup.m_anyHitShader ? _hitGroup.m_anyHitShader.getObject() : nullptr;
    const gpu::IntersectionShaderPointer intersectionShader = _hitGroup.m_intersectionShader ? _hitGroup.m_intersectionShader.getObject() : nullptr;

    RayTracingHitGroupKey key;
    key.m_type = _hitGroup.m_intersectionShader ? gpu::RayTracingHitGroupType::ProceduralPrimitive : gpu::RayTracingHitGroupType::Triangles;
    key.m_closestHitShader = closestHitShader.getObject();
    key.m_anyHitShader = anyHitShader.getObject();
    key.m_intersectionShader = intersectionShader.getObject();

    return key;
}
ego::gpu::RayTracingHitGroupDesc ego::render::RayTracingRenderPass::MakeHitGroupDesc(const RayTracingMaterialHitGroup& _hitGroup)
{
    gpu::RayTracingHitGroupDesc desc;
    desc.m_type = _hitGroup.m_intersectionShader ? gpu::RayTracingHitGroupType::ProceduralPrimitive : gpu::RayTracingHitGroupType::Triangles;
    desc.m_closestHitShader = _hitGroup.m_closestHitShader ? _hitGroup.m_closestHitShader.getObject() : nullptr;
    desc.m_anyHitShader = _hitGroup.m_anyHitShader ? _hitGroup.m_anyHitShader.getObject() : nullptr;
    desc.m_intersectionShader = _hitGroup.m_intersectionShader ? _hitGroup.m_intersectionShader.getObject() : nullptr;

    return desc;
}
