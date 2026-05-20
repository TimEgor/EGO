#include "CommandList.h"

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
