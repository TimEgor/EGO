#include <algorithm>

#include "CommandList.h"

const std::vector<ego::gpu::GpuTaskReference>& ego::gpu::CommandList::getGpuWaits() const
{
    return m_gpuWaits;
}

void ego::gpu::CommandList::addGpuWait(const GpuTaskReference& _task)
{
    if (!_task || _task->isComplete())
    {
        return;
    }

    if (std::find(m_gpuWaits.begin(), m_gpuWaits.end(), _task) == m_gpuWaits.end())
    {
        m_gpuWaits.push_back(_task);
    }
}

void ego::gpu::CommandList::addResourceGpuWait(const GraphicResourceReference& _resource)
{
    if (_resource)
    {
        addGpuWait(_resource->getLastWriteTask());
    }
}

void ego::gpu::CommandList::clearGpuWaits()
{
    m_gpuWaits.clear();
}

ego::gpu::CommandType ego::gpu::CopyCommandList::getCommandType() const
{
    return CommandType::Copy;
}

ego::gpu::CommandType ego::gpu::ComputeCommandList::getCommandType() const
{
    return CommandType::Compute;
}

ego::gpu::CommandType ego::gpu::GraphicCommandList::getCommandType() const
{
    return CommandType::Graphic;
}
