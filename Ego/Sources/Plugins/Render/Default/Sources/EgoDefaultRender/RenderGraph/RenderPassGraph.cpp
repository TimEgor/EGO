#include "RenderPassGraph.h"

#include "EgoCore/Assert/Assert.h"

ego::render::RenderPassGraphNode::RenderPassGraphNode(const char* _name, RenderPass& _pass)
    : m_name(_name),
      m_pass(_pass)
{
}

void ego::render::RenderPassGraph::addPass(const char* _name, RenderPass& _pass)
{
    m_nodes.emplace_back(_name, _pass);
}

void ego::render::RenderPassGraph::clear()
{
    m_commandLists.clear();
    m_schedule.clear();
    m_nodes.clear();
}

size_t ego::render::RenderPassGraph::getPassCount() const
{
    return m_schedule.empty() ? m_nodes.size() : m_schedule.size();
}

const ego::render::RenderPassGraphNode& ego::render::RenderPassGraph::getPassNode(size_t _index) const
{
    EGO_ASSERT(_index < m_nodes.size());
    return m_nodes[_index];
}

bool ego::render::RenderPassGraph::prepareCommandLists(GraphicDevice& _graphicDevice)
{
    const size_t passCount = getPassCount();
    if (m_commandLists.size() == passCount)
    {
        bool hasValidCommandLists = true;
        for (const RenderGraphicCommandList& commandList : m_commandLists)
        {
            hasValidCommandLists = hasValidCommandLists && static_cast<bool>(commandList);
        }

        if (hasValidCommandLists)
        {
            return true;
        }
    }

    m_commandLists.clear();
    m_commandLists.reserve(passCount);

    for (size_t passIndex = 0; passIndex < passCount; ++passIndex)
    {
        RenderGraphicCommandList commandList = _graphicDevice.createGraphicCommandList();
        if (!commandList)
        {
            return false;
        }

        m_commandLists.push_back(commandList);
    }

    return true;
}

const std::vector<ego::render::RenderGraphicCommandList>& ego::render::RenderPassGraph::getCommandLists() const
{
    return m_commandLists;
}

bool ego::render::RenderPassGraph::compile()
{
    m_schedule.clear();
    m_schedule.reserve(m_nodes.size());

    for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
    {
        RenderPassGraphNode& node = m_nodes[nodeIndex];
        RenderPassBuilder builder(node.m_name);
        node.m_pass.get().declare(builder);
        node.m_resourceUsages = builder.getResourceUsages();
        m_schedule.push_back(nodeIndex);
    }

    return true;
}

bool ego::render::RenderPassGraph::init(RenderPassInitContext& _context)
{
    for (RenderPassGraphNode& node : m_nodes)
    {
        if (!node.m_pass.get().init(_context))
        {
            return false;
        }
    }

    return true;
}

void ego::render::RenderPassGraph::release()
{
    for (RenderPassGraphNode& node : m_nodes)
    {
        node.m_pass.get().release();
    }
}

void ego::render::RenderPassGraph::clearResources()
{
    for (RenderPassGraphNode& node : m_nodes)
    {
        node.m_pass.get().clearResources();
    }
}

bool ego::render::RenderPassGraph::prepare(RenderPassPrepareContext& _context)
{
    for (size_t nodeIndex : m_schedule)
    {
        RenderPassGraphNode& node = m_nodes[nodeIndex];
        if (!node.m_pass.get().prepare(_context))
        {
            return false;
        }
    }

    return true;
}

bool ego::render::RenderPassGraph::execute(RenderPassExecuteContext& _context, const RenderPassGraphFinalizer& _finalizer)
{
    if (m_commandLists.size() != m_schedule.size())
    {
        return false;
    }

    for (const RenderGraphicCommandList& commandList : m_commandLists)
    {
        if (!commandList)
        {
            return false;
        }
    }

    for (size_t scheduleIndex = 0; scheduleIndex < m_schedule.size(); ++scheduleIndex)
    {
        const RenderGraphicCommandList& commandList = m_commandLists[scheduleIndex];
        RenderPassGraphNode& node = m_nodes[m_schedule[scheduleIndex]];

        commandList->begin();

        RenderPassExecuteContext passContext{
            _context.m_graphicDevice,
            _context.m_pipelineStateCache,
            commandList,
            _context.m_renderTarget,
            _context.m_scene,
            _context.m_shaderData,
            _context.m_settings};
        node.m_pass.get().execute(passContext);

        if (_finalizer && scheduleIndex + 1 == m_schedule.size())
        {
            _finalizer(commandList);
        }

        commandList->end();
    }

    return true;
}
