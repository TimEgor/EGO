#include "DebugElementBufferPool.h"

#include <limits>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"

ego::render::DebugElementBufferPool::~DebugElementBufferPool()
{
    release();
}

bool ego::render::DebugElementBufferPool::init(
    const BufferDesc& _bufferDesc,
    uint32_t _elementCountPerBuffer
)
{
    EGO_CHECK_RETURN_FALSE(_bufferDesc.m_stride > 0);
    EGO_CHECK_RETURN_FALSE(_elementCountPerBuffer > 0);
    EGO_CHECK_RETURN_FALSE(
        _elementCountPerBuffer <= (std::numeric_limits<uint32_t>::max)() / _bufferDesc.m_stride
    );

    release();

    m_bufferDesc.m_size = _bufferDesc.m_stride * _elementCountPerBuffer;
    m_bufferDesc.m_stride = _bufferDesc.m_stride;
    m_bufferDesc.m_access = _bufferDesc.m_access;
    m_bufferDesc.m_usage = _bufferDesc.m_usage;
    m_elementCountPerBuffer = _elementCountPerBuffer;
    return true;
}

void ego::render::DebugElementBufferPool::release()
{
    m_buffers = BufferCollection();
    m_bufferDesc = gpu::BufferDesc();
    m_elementCountPerBuffer = 0;
    m_activeBufferCount = 0;
}

bool ego::render::DebugElementBufferPool::updateBuffers(
    float _deltaTime,
    uint32_t _requiredElementCount
)
{
    addDelayTime(_deltaTime);
    EGO_CHECK_RETURN_FALSE(prepareSpace(_requiredElementCount));
    freeUnusedSpace();
    return true;
}

void ego::render::DebugElementBufferPool::updateBuffers(float _deltaTime)
{
    addDelayTime(_deltaTime);
}

bool ego::render::DebugElementBufferPool::prepareSpace(uint32_t _requiredElementCount)
{
    EGO_CHECK_RETURN_FALSE(m_elementCountPerBuffer > 0);

    const uint32_t requiredBufferCount = CalculateRequiredBufferCount(
        _requiredElementCount,
        m_elementCountPerBuffer
    );
    EGO_ASSERT(m_activeBufferCount <= m_buffers.size());

    if (requiredBufferCount > m_activeBufferCount)
    {
        EGO_CHECK_RETURN_FALSE(activateBuffers(requiredBufferCount));
    }
    else if (requiredBufferCount < m_activeBufferCount)
    {
        deactivateBuffers(requiredBufferCount);
    }

    return true;
}

void ego::render::DebugElementBufferPool::freeUnusedSpace()
{
    const uint32_t bufferCount = static_cast<uint32_t>(m_buffers.size());
    EGO_ASSERT(m_activeBufferCount <= bufferCount);
    if (m_activeBufferCount >= bufferCount)
    {
        return;
    }

    uint32_t releasingBufferIndex = bufferCount;

    for (uint32_t bufferIndex = m_activeBufferCount; bufferIndex < bufferCount; ++bufferIndex)
    {
        const BufferData& bufferData = m_buffers[bufferIndex];
        if (bufferData.m_unusedTime >= BufferReleasingDelay)
        {
            releasingBufferIndex = bufferIndex;
            break;
        }
    }

    m_buffers.resize(releasingBufferIndex);
}

uint32_t ego::render::DebugElementBufferPool::getBufferCount() const
{
    EGO_ASSERT(m_activeBufferCount <= m_buffers.size());
    return m_activeBufferCount;
}

const ego::render::RenderBuffer& ego::render::DebugElementBufferPool::getBuffer(uint32_t _bufferIndex) const
{
    EGO_ASSERT(_bufferIndex < getBufferCount());
    return m_buffers[_bufferIndex].m_buffer;
}

ego::render::RenderBuffer ego::render::DebugElementBufferPool::createNewBuffer() const
{
    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();
    return graphicDevice.createBuffer(m_bufferDesc);
}

void ego::render::DebugElementBufferPool::addDelayTime(float _deltaTime)
{
    const uint32_t bufferCount = static_cast<uint32_t>(m_buffers.size());
    EGO_ASSERT(m_activeBufferCount <= bufferCount);

    for (uint32_t bufferIndex = m_activeBufferCount; bufferIndex < bufferCount; ++bufferIndex)
    {
        BufferData& bufferData = m_buffers[bufferIndex];
        bufferData.m_unusedTime += _deltaTime;
    }
}

bool ego::render::DebugElementBufferPool::activateBuffers(uint32_t _requiredBufferCount)
{
    const uint32_t currentBufferCount = static_cast<uint32_t>(m_buffers.size());
    if (_requiredBufferCount > currentBufferCount)
    {
        m_buffers.reserve(_requiredBufferCount);
        for (uint32_t bufferIndex = currentBufferCount; bufferIndex < _requiredBufferCount; ++bufferIndex)
        {
            BufferData bufferData;
            bufferData.m_buffer = createNewBuffer();
            EGO_CHECK_RETURN_FALSE(bufferData.m_buffer);
            m_buffers.push_back(bufferData);
        }
    }

    for (uint32_t bufferIndex = m_activeBufferCount; bufferIndex < _requiredBufferCount; ++bufferIndex)
    {
        BufferData& bufferData = m_buffers[bufferIndex];
        bufferData.m_unusedTime = 0.0f;
    }

    m_activeBufferCount = _requiredBufferCount;
    return true;
}

void ego::render::DebugElementBufferPool::deactivateBuffers(uint32_t _requiredBufferCount)
{
    EGO_ASSERT(_requiredBufferCount <= m_activeBufferCount);

    for (uint32_t bufferIndex = _requiredBufferCount; bufferIndex < m_activeBufferCount; ++bufferIndex)
    {
        BufferData& bufferData = m_buffers[bufferIndex];
        bufferData.m_unusedTime = 0.0f;
    }

    m_activeBufferCount = _requiredBufferCount;
}

uint32_t ego::render::DebugElementBufferPool::CalculateRequiredBufferCount(
    uint32_t _requiredElementCount,
    uint32_t _elementCountPerBuffer
)
{
    if (_requiredElementCount == 0)
    {
        return 0;
    }

    return (_requiredElementCount - 1) / _elementCountPerBuffer + 1;
}

ego::render::DebugElementBufferPoolIterator::DebugElementBufferPoolIterator(
    const DebugElementBufferPool& _bufferPool,
    bool _iterateOnInit
)
    : m_bufferPool(_bufferPool)
{
    if (_iterateOnInit)
    {
        next();
    }
}

ego::render::DebugElementBufferPoolIterator::~DebugElementBufferPoolIterator()
{
    unmapCurrentBuffer();
}

bool ego::render::DebugElementBufferPoolIterator::isInInitialState() const
{
    return
        !m_iterationStarted &&
        m_currentElement == nullptr &&
        m_currentBufferIndex == 0 &&
        m_currentElementIndex == 0;
}

bool ego::render::DebugElementBufferPoolIterator::next()
{
    if (!m_iterationStarted)
    {
        m_iterationStarted = true;
        return mapCurrentBuffer();
    }

    EGO_CHECK_RETURN_FALSE(m_currentBufferData);

    const uint32_t elementCountPerBuffer = m_bufferPool.getElementCountPerBuffer();
    EGO_CHECK_RETURN_FALSE(elementCountPerBuffer > 0);

    ++m_currentElementIndex;
    if (m_currentElement && m_currentElementIndex >= elementCountPerBuffer)
    {
        unmapCurrentBuffer();
        ++m_currentBufferIndex;
        return mapCurrentBuffer();
    }

    m_currentElement = static_cast<uint8_t*>(m_currentBufferData) +
        m_currentElementIndex * m_bufferPool.getBufferDesc().m_stride;
    return true;
}

bool ego::render::DebugElementBufferPoolIterator::mapCurrentBuffer()
{
    if (m_currentBufferIndex >= m_bufferPool.getBufferCount())
    {
        return false;
    }

    m_currentBuffer = m_bufferPool.getBuffer(m_currentBufferIndex);
    EGO_CHECK_RETURN_FALSE(m_currentBuffer);

    m_currentBufferData = m_currentBuffer->map(0, m_bufferPool.getBufferDesc().m_size);
    EGO_CHECK_RETURN_FALSE(m_currentBufferData);

    m_currentElement = m_currentBufferData;
    m_currentElementIndex = 0;
    return true;
}

void ego::render::DebugElementBufferPoolIterator::unmapCurrentBuffer()
{
    if (!m_currentBufferData)
    {
        return;
    }

    if (m_currentBuffer)
    {
        m_currentBuffer->unmap(0, m_bufferPool.getBufferDesc().m_size);
    }

    m_currentBuffer = nullptr;
    m_currentBufferData = nullptr;
    m_currentElement = nullptr;
}
