#include "CommandQueue.h"

ego::gpu::CommandQueue::CommandQueue(const CommandQueueDesc& _desc)
    : m_desc(_desc)
{
}

ego::gpu::CommandType ego::gpu::CommandQueue::getCommandType() const
{
    return m_desc.m_type;
}

const ego::gpu::CommandQueueDesc& ego::gpu::CommandQueue::getDesc() const
{
    return m_desc;
}
