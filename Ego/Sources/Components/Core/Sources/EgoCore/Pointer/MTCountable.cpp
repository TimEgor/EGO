#include "MTCountable.h"

#include "EgoCore/Assert/Assert.h"

ego::MTCountable::MTCountable()
    : m_count(0),
      m_destroyFunction(nullptr)
{
}

ego::MTCountable::~MTCountable()
{
    EGO_ASSERT(m_count.load() == 0);
}

void ego::MTCountable::addReference()
{
    EGO_ASSERT(m_destroyFunction != nullptr);

    incrementCount();
}

void ego::MTCountable::releaseReference()
{
    decrementCount();
}

uint32_t ego::MTCountable::getReferenceCount() const
{
    return m_count.load();
}

void ego::MTCountable::incrementCount()
{
    m_count.fetch_add(1);
}

void ego::MTCountable::decrementCount()
{
    const uint32_t oldValue = m_count.fetch_sub(1);

    EGO_ASSERT(oldValue > 0);

    if (oldValue == 1)
    {
        EGO_ASSERT(m_destroyFunction != nullptr);

        m_destroyFunction(this);
    }
}

void ego::MTCountable::setDestroyFunction(DestroyFunction _destroyFunction)
{
    EGO_ASSERT(_destroyFunction != nullptr);
    EGO_ASSERT(m_destroyFunction == nullptr);
    EGO_ASSERT(m_count == 0);

    m_destroyFunction = _destroyFunction;
}
