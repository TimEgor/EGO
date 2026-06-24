#include "MTCountable.h"

#include "EgoCore/Assert/AssertCore.h"

ego::MTCountable::MTCountable(uint32_t _initialCounterVal)
    : m_count(_initialCounterVal)
{
}

ego::MTCountable::~MTCountable()
{
    EGO_ASSERT(m_count.load() == 0);
}

void ego::MTCountable::addReference()
{
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
    if (m_count.fetch_add(-1) == 1)
    {
        onRelease();
    }
}

void ego::STDDestroyMTCountable::onRelease()
{
    delete this;
}
