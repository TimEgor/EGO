#include "DebugRenderPass.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/GeneralResources/XmlResource.h"
#include "EgoResource/ResourceSubsystem.h"
#include "EgoResource/ResourceController.h"

#include "EgoGraphicHardware/Resources/ShaderResource.h"

#include "EgoEngine/Graphic/Render/RenderResourceObject.h"

#include "EgoDefaultRender/DefaultRenderBindingLayout.h"

namespace ego::render
{
    constexpr auto DebugRenderPassConfigPath = "Configs/DebugRenderPass.xml";

    static bool ReadDebugRenderPassRequiredFileName(const XmlNode& _node, const char* _childName, FileName& _fileName)
    {
        const std::string value = _node.getChildValueOr<std::string>(_childName, std::string());
        EGO_CHECK_RETURN_FALSE(!value.empty());

        _fileName = FileName(value);
        return static_cast<bool>(_fileName);
    }

    static bool LoadDebugRenderPassGraphicShaders(const XmlNode& _parentNode, const char* _nodeName, DefaultRenderDebugDraw::ShaderInitData& _initData)
    {
        const XmlNode shaderNode = _parentNode.getChild(_nodeName);
        EGO_CHECK_RETURN_FALSE(shaderNode);

        FileName vertexShaderPath;
        FileName pixelShaderPath;
        EGO_CHECK_RETURN_FALSE(ReadDebugRenderPassRequiredFileName(shaderNode, "VertexShader", vertexShaderPath));
        EGO_CHECK_RETURN_FALSE(ReadDebugRenderPassRequiredFileName(shaderNode, "PixelShader", pixelShaderPath));

        ResourceController& resourceController = GetResourceSubsystem().getResourceController();

        const gpu::VertexShaderResourcePointer vertexShaderResource = resourceController.load<gpu::VertexShaderResource>(vertexShaderPath);
        const gpu::PixelShaderResourcePointer pixelShaderResource = resourceController.load<gpu::PixelShaderResource>(pixelShaderPath);
        EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());
        EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

        _initData.m_vertexShader = CreateVertexShaderHandler(vertexShaderResource);
        _initData.m_pixelShader = CreatePixelShaderHandler(pixelShaderResource);

        return _initData.m_vertexShader && _initData.m_pixelShader;
    }

    static bool LoadDebugRenderPassInitDataFromNode(const XmlNode& _configNode, DefaultRenderDebugDraw::InitData& _initData)
    {
        EGO_CHECK_RETURN_FALSE(_configNode && _configNode.getNameView() == "DebugRenderPass");

        return LoadDebugRenderPassGraphicShaders(_configNode, "Point", _initData.m_point) && LoadDebugRenderPassGraphicShaders(_configNode, "Line", _initData.m_line);
    }
} // namespace ego::render

bool ego::render::DebugRenderPass::init(RenderPassInitContext& _context)
{
    DefaultRenderDebugDraw::InitData debugDrawInitData;
    EGO_CHECK_RETURN_FALSE(LoadDebugDrawInitData(debugDrawInitData));

    m_bindingLayout = CreateDefaultRenderBindlessBindingLayout(_context.m_graphicDevice);
    EGO_CHECK_RETURN_FALSE(m_bindingLayout);

    return m_debugDraw.init(m_bindingLayout, _context.m_renderTargetFormat, debugDrawInitData);
}

void ego::render::DebugRenderPass::release()
{
    m_debugDraw.release();
    m_bindingLayout = nullptr;
}

void ego::render::DebugRenderPass::clearResources()
{
    m_debugDraw.clearResources();
}

void ego::render::DebugRenderPass::declare(RenderPassBuilder& _builder)
{
    _builder.writeTexture("DefaultRenderTarget", gpu::GraphicResourceState::RenderTarget, gpu::GraphicResourceViewType::RenderTarget);
    _builder.readBuffer("CameraShaderData");
}

bool ego::render::DebugRenderPass::prepare(RenderPassPrepareContext& _context)
{
    return m_debugDraw.prepare(_context.m_deltaTime);
}

void ego::render::DebugRenderPass::execute(RenderPassExecuteContext& _context)
{
    if (!_context.m_commandList || !_context.m_renderTarget.getRenderTargetView())
    {
        return;
    }

    _context.m_renderTarget.transition(_context.m_commandList, gpu::GraphicResourceState::RenderTarget);

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _context.m_renderTarget.getRenderTargetView().getObject();
    colorAttachment.m_loadOperation = gpu::AttachmentLoadOperation::Load;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = _context.m_settings.m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = _context.m_renderTarget.getResolution();

    _context.m_commandList->beginRendering(renderingDesc);
    SetupTargetViewport(_context);
    m_debugDraw.render(_context.m_graphicDevice, _context.m_pipelineStateCache, _context.m_commandList, _context.m_shaderData.getCameraShaderDataView());
    _context.m_commandList->endRendering();
}

void ego::render::DebugRenderPass::drawPoint(const DebugDrawPointData& _point)
{
    m_debugDraw.drawPoint(_point);
}

void ego::render::DebugRenderPass::drawLine(const DebugDrawLineData& _line)
{
    m_debugDraw.drawLine(_line);
}

bool ego::render::DebugRenderPass::LoadDebugDrawInitData(DefaultRenderDebugDraw::InitData& _initData)
{
    ResourceController& resourceController = GetResourceSubsystem().getResourceController();
    const XmlResourcePointer configResource = resourceController.load<XmlResource>(DebugRenderPassConfigPath);
    return configResource && configResource->isLoaded() && LoadDebugRenderPassInitDataFromNode(configResource->getRootNode(), _initData);
}

void ego::render::DebugRenderPass::SetupTargetViewport(RenderPassExecuteContext& _context)
{
    if (!_context.m_renderTarget.getTexture())
    {
        EGO_ASSERT_FAIL();
        return;
    }

    const gpu::Texture2DSize& resolution = _context.m_renderTarget.getResolution();

    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = static_cast<float>(resolution.m_x);
    viewportDesc.m_height = static_cast<float>(resolution.m_y);
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    _context.m_commandList->setViewport(viewportDesc);

    gpu::ScissorRectDesc scissorRectDesc;
    scissorRectDesc.m_right = static_cast<int32_t>(resolution.m_x);
    scissorRectDesc.m_bottom = static_cast<int32_t>(resolution.m_y);
    _context.m_commandList->setScissorRect(scissorRectDesc);
}
