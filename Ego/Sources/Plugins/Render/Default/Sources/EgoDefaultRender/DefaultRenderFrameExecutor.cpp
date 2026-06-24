#include "DefaultRenderFrameExecutor.h"

#include "EgoCore/UtilsMacros.h"

bool ego::render::DefaultRenderFrameExecutor::init(GraphicDevice& _graphicDevice, const gpu::CommandQueueReference& _commandQueue)
{
    EGO_CHECK_RETURN_FALSE(_commandQueue && _commandQueue->getCommandType() == gpu::CommandType::Graphic);
    m_commandQueue = _commandQueue;

    m_presentCommandList = _graphicDevice.createGraphicCommandList();
    EGO_CHECK_RETURN_FALSE(m_presentCommandList);

    m_frameFence = _graphicDevice.createFence();
    EGO_CHECK_RETURN_FALSE(m_frameFence);

    return true;
}

void ego::render::DefaultRenderFrameExecutor::release()
{
    m_presentCommandList = nullptr;
    m_frameFence = nullptr;
    m_frameFenceValue = 0;
    m_commandQueue = nullptr;
}

void ego::render::DefaultRenderFrameExecutor::wait()
{
    if (m_frameFence)
    {
        m_frameFence->waitValue(m_frameFenceValue);
        return;
    }

    if (m_commandQueue)
    {
        m_commandQueue->waitIdle();
    }
}

void ego::render::DefaultRenderFrameExecutor::submitCommandList(const RenderGraphicCommandList& _commandList)
{
    if (!m_commandQueue || !_commandList)
    {
        return;
    }

    m_commandQueue->execute(_commandList.getObject());
    signalFrameFence();
}

void ego::render::DefaultRenderFrameExecutor::submitCommandLists(const std::vector<RenderGraphicCommandList>& _commandLists)
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
    signalFrameFence();
}

bool ego::render::DefaultRenderFrameExecutor::isValid() const
{
    return m_commandQueue && m_presentCommandList;
}

const ego::render::RenderGraphicCommandList& ego::render::DefaultRenderFrameExecutor::getPresentCommandList() const
{
    return m_presentCommandList;
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
