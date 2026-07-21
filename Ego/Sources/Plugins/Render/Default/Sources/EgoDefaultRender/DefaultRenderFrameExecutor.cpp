#include "DefaultRenderFrameExecutor.h"

#include <utility>

#include "EgoCore/UtilsMacros.h"

bool ego::render::DefaultRenderFrameExecutor::init(GraphicDevice& _graphicDevice, const gpu::CommandQueueReference& _commandQueue)
{
    EGO_CHECK_RETURN_FALSE(_commandQueue && _commandQueue->getCommandType() == gpu::CommandType::Graphic);
    m_commandQueue = _commandQueue;

    m_frameFence = _graphicDevice.createFence();
    EGO_CHECK_RETURN_FALSE(m_frameFence);

    return true;
}

void ego::render::DefaultRenderFrameExecutor::release()
{
    wait();
    m_frameResources.clear();
    m_frameFence = nullptr;
    m_frameFenceValue = 0;
    m_commandQueue = nullptr;
}

void ego::render::DefaultRenderFrameExecutor::wait()
{
    if (m_frameFence)
    {
        m_frameFence->waitValue(m_frameFenceValue);
        m_frameResources.clear();
        return;
    }

    if (m_commandQueue)
    {
        m_commandQueue->waitIdle();
    }

    m_frameResources.clear();
}

void ego::render::DefaultRenderFrameExecutor::submitCommandLists(
    const std::vector<RenderGraphicCommandList>& _commandLists,
    std::vector<gpu::GraphicObjectReference>&& _frameResources)
{
    if (!m_commandQueue || _commandLists.empty())
    {
        return;
    }

    std::vector<gpu::CommandListReference> commandLists;
    commandLists.reserve(_commandLists.size());

    for (const RenderGraphicCommandList& commandList : _commandLists)
    {
        if (commandList)
        {
            commandLists.push_back(commandList.getObject());
        }
    }

    if (commandLists.empty())
    {
        return;
    }

    m_commandQueue->execute(commandLists);
    m_frameResources = std::move(_frameResources);
    signalFrameFence();
}

bool ego::render::DefaultRenderFrameExecutor::isValid() const
{
    return m_commandQueue && m_frameFence;
}

void ego::render::DefaultRenderFrameExecutor::signalFrameFence()
{
    if (!m_frameFence || !m_commandQueue)
    {
        return;
    }

    ++m_frameFenceValue;
    m_commandQueue->signal(m_frameFence.getObject(), m_frameFenceValue);
}
