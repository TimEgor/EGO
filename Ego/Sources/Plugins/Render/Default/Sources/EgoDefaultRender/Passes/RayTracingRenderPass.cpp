#include "RayTracingRenderPass.h"

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/RenderResourceObject.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ShaderResource.h"
#include "EgoEngine/Resources/GeneralResources/XmlResource.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

#include "EgoDefaultRender/DefaultRenderBindingLayout.h"
#include "EgoDefaultRender/DefaultRenderConstants.h"
#include "EgoDefaultRender/RenderShaderData.h"

namespace ego::render
{
    constexpr auto RayTracingRenderPassConfigPath = "Configs/RayTracingRenderPass.xml";

    struct RayTracingRenderPassShaderConfig final
    {
        FileName m_rayGenerationShaderPath;
        FileName m_missShaderPath;
        FileName m_closestHitShaderPath;
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
               ReadRayTracingRenderPassRequiredFileName(_configNode, "MissShader", _config.m_missShaderPath) &&
               ReadRayTracingRenderPassRequiredFileName(_configNode, "ClosestHitShader", _config.m_closestHitShaderPath);
    }

    static bool LoadRayTracingRenderPassShaderConfig(RayTracingRenderPassShaderConfig& _config)
    {
        ResourceController& resourceController = engine::GetEngine().getResourceController();
        const XmlResourcePointer configResource = resourceController.load<XmlResource>(RayTracingRenderPassConfigPath);
        return configResource && configResource->isLoaded() && ParseRayTracingRenderPassShaderConfig(configResource->getRootNode(), _config);
    }
} // namespace ego::render

bool ego::render::RayTracingRenderPass::init(RenderPassInitContext& _context)
{
    EGO_CHECK_RETURN_FALSE(loadShaders());

    m_bindingLayout = CreateDefaultRenderBindlessBindingLayout(_context.m_graphicDevice);
    EGO_CHECK_RETURN_FALSE(m_bindingLayout);

    return initPipeline(_context.m_graphicDevice, m_bindingLayout);
}

void ego::render::RayTracingRenderPass::release()
{
    clearResources();

    m_pipeline = nullptr;
    m_bindingLayout = nullptr;
    m_closestHitShader = nullptr;
    m_missShader = nullptr;
    m_rayGenerationShader = nullptr;
}

void ego::render::RayTracingRenderPass::clearResources()
{
    m_sceneAccelerationStructureView = nullptr;
    m_sceneAccelerationStructure = nullptr;
    m_geometryCache.clear();
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
    m_geometryCache.beginFrame();

    const DefaultRenderScene::ItemCollection& renderItems = _context.m_scene.getItems();
    if (renderItems.empty())
    {
        m_geometryCache.removeUnused();
        return true;
    }

    gpu::InstanceAccelerationStructureBuildDesc sceneDesc;
    sceneDesc.m_instances.reserve(renderItems.size());

    for (const DefaultRenderItem& item : renderItems)
    {
        if (!item.m_mesh)
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
        instanceDesc.m_hitGroupIndex = 0;

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
    if (!_context.m_commandList || !m_pipeline || !_context.m_shaderData.getCameraShaderDataView() || !m_sceneAccelerationStructureView ||
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
    _context.m_commandList->setPipeline(m_pipeline.getObject());
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

    ResourceController& resourceController = engine::GetEngine().getResourceController();

    const gpu::RayGenerationShaderResourcePointer rayGenerationShaderResource = resourceController.load<gpu::RayGenerationShaderResource>(config.m_rayGenerationShaderPath);
    const gpu::MissShaderResourcePointer missShaderResource = resourceController.load<gpu::MissShaderResource>(config.m_missShaderPath);
    const gpu::ClosestHitShaderResourcePointer closestHitShaderResource = resourceController.load<gpu::ClosestHitShaderResource>(config.m_closestHitShaderPath);
    EGO_CHECK_RETURN_FALSE(rayGenerationShaderResource && rayGenerationShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(missShaderResource && missShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(closestHitShaderResource && closestHitShaderResource->isLoaded());

    m_rayGenerationShader = CreateRayGenerationShaderHandler(rayGenerationShaderResource);
    m_missShader = CreateMissShaderHandler(missShaderResource);
    m_closestHitShader = CreateClosestHitShaderHandler(closestHitShaderResource);

    return m_rayGenerationShader && m_missShader && m_closestHitShader;
}

bool ego::render::RayTracingRenderPass::initPipeline(GraphicDevice& _graphicDevice, const RenderBindingLayout& _bindingLayout)
{
    EGO_CHECK_RETURN_FALSE(m_rayGenerationShader && m_missShader && m_closestHitShader);

    gpu::RayTracingPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = _bindingLayout.getObject();
    pipelineDesc.m_rayGenerationShader = m_rayGenerationShader.getObject();
    pipelineDesc.m_missShader = m_missShader.getObject();
    pipelineDesc.m_closestHitShader = m_closestHitShader.getObject();
    pipelineDesc.m_maxPayloadSize = DefaultRenderRayTracingPayloadSize;
    pipelineDesc.m_maxAttributeSize = DefaultRenderRayTracingAttributeSize;
    pipelineDesc.m_maxRecursionDepth = 1;

    m_pipeline = _graphicDevice.createRayTracingPipeline(pipelineDesc);
    return static_cast<bool>(m_pipeline);
}
