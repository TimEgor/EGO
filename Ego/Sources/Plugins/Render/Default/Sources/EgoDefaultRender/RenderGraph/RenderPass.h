#pragma once

#include "RenderPassBuilder.h"
#include "RenderPassContext.h"

namespace ego::render
{
    class RenderPass
    {
    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual bool init(RenderPassInitContext& _context) = 0;
        virtual void release() = 0;
        virtual void clearResources() = 0;
        virtual void declare(RenderPassBuilder& _builder) = 0;
        virtual bool prepare(RenderPassPrepareContext& _context) = 0;
        virtual void execute(RenderPassExecuteContext& _context) = 0;
    };
} // namespace ego::render
