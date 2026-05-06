#include "AssertCore.h"

void ego::AssertCore::setGenerator(AssertGenerator* _generator)
{
    if (m_generator)
    {
        EGO_ASSERT_FAIL_MESSAGE("Assert generator has been already set.");
        return;
    }

    m_generator = _generator;
}

ego::AssertGenerator* ego::AssertCore::getGenerator() const
{
    return m_generator;
}
