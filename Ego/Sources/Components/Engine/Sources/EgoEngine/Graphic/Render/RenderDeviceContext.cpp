#include "RenderDeviceContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

bool ego::render::RenderDeviceContext::init(GraphicDevice& _graphicDevice)
{
    release();

    gpu::CommandQueueDesc commandQueueDesc;
    commandQueueDesc.m_type = gpu::CommandType::Graphic;
    commandQueueDesc.m_supportsPresentation = true;

    m_graphicCommandQueue = _graphicDevice.createCommandQueue(commandQueueDesc);
    EGO_CHECK_RETURN_FALSE(m_graphicCommandQueue);

    return true;
}

void ego::render::RenderDeviceContext::release()
{
    m_graphicCommandQueue = nullptr;
}

bool ego::render::RenderDeviceContext::isValid() const
{
    return m_graphicCommandQueue;
}

const ego::gpu::CommandQueueReference& ego::render::RenderDeviceContext::getGraphicCommandQueue() const
{
    EGO_ASSERT(m_graphicCommandQueue);
    return m_graphicCommandQueue;
}
