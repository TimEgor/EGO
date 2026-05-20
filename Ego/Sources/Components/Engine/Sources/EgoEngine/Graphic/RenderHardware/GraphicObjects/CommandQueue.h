#pragma once

#include <vector>

#include "CommandList.h"
#include "Fence.h"
#include "GraphicObject.h"

namespace ego::gpu
{
    struct CommandQueueDesc final
    {
        CommandType m_type = CommandType::Graphic;
        bool m_supportsPresentation = false;
    };

    class CommandQueue : public GraphicObject
    {
    public:
        CommandQueue(const CommandQueueDesc& _desc);

        CommandType getCommandType() const;
        const CommandQueueDesc& getDesc() const;

        virtual void execute(const CommandListPointer& _commandList) = 0;
        virtual void execute(const std::vector<CommandListPointer>& _commandLists) = 0;

        virtual void signal(const FencePointer& _fence, Fence::FenceValue _value) = 0;
        virtual void wait(const FencePointer& _fence, Fence::FenceValue _value) = 0;
        virtual void waitIdle() = 0;

    private:
        CommandQueueDesc m_desc;
    };

    EGO_POINTER(CommandQueue);
}
