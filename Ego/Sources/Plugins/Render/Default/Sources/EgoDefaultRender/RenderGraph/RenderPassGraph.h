#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "RenderPass.h"

namespace ego::render
{
    using RenderPassGraphFinalizer = std::function<void(const RenderGraphicCommandList&)>;

    struct RenderPassGraphNode final
    {
        RenderPassGraphNode(const char* _name, RenderPass& _pass);

        const char* m_name = nullptr;
        std::reference_wrapper<RenderPass> m_pass;
        std::vector<RenderGraphResourceUsage> m_resourceUsages;
    };

    class RenderPassGraph final
    {
    public:
        RenderPassGraph() = default;

        void addPass(const char* _name, RenderPass& _pass);
        void clear();
        size_t getPassCount() const;
        const RenderPassGraphNode& getPassNode(size_t _index) const;
        bool prepareCommandLists(GraphicDevice& _graphicDevice);
        const std::vector<RenderGraphicCommandList>& getCommandLists() const;

        bool compile();
        bool init(RenderPassInitContext& _context);
        void release();
        void clearResources();
        bool prepare(RenderPassPrepareContext& _context);
        bool execute(RenderPassExecuteContext& _context, const RenderPassGraphFinalizer& _finalizer = RenderPassGraphFinalizer());

    private:
        std::vector<RenderPassGraphNode> m_nodes;
        std::vector<size_t> m_schedule;
        std::vector<RenderGraphicCommandList> m_commandLists;
    };
} // namespace ego::render
