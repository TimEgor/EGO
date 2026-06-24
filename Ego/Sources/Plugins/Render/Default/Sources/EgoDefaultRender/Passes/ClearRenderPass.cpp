#include "ClearRenderPass.h"

bool ego::render::ClearRenderPass::init(RenderPassInitContext&)
{
    return true;
}

void ego::render::ClearRenderPass::release() {}

void ego::render::ClearRenderPass::clearResources() {}

void ego::render::ClearRenderPass::declare(RenderPassBuilder& _builder)
{
    _builder.writeTexture("DefaultRenderTarget", gpu::GraphicResourceState::RenderTarget, gpu::GraphicResourceViewType::RenderTarget);
}

bool ego::render::ClearRenderPass::prepare(RenderPassPrepareContext&)
{
    return true;
}

void ego::render::ClearRenderPass::execute(RenderPassExecuteContext& _context)
{
    if (!_context.m_settings.m_clearEnabled || !_context.m_commandList || !_context.m_renderTarget.getRenderTargetView())
    {
        return;
    }

    _context.m_renderTarget.transition(_context.m_commandList, gpu::GraphicResourceState::RenderTarget);

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _context.m_renderTarget.getRenderTargetView().getObject();
    colorAttachment.m_loadOperation = gpu::AttachmentLoadOperation::Clear;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = _context.m_settings.m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = _context.m_renderTarget.getResolution();

    _context.m_commandList->beginRendering(renderingDesc);
    _context.m_commandList->endRendering();
}
