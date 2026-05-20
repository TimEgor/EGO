#include "AssertCore.h"

void ego::AssertCore::setGenerator(const AssertGeneratorPointer& _generator)
{
    if (m_generator)
    {
        EGO_ASSERT_FAIL_MESSAGE("Assert generator has been already set.");
        return;
    }

    m_generator = _generator;
}

ego::AssertGeneratorPointer ego::AssertCore::getGenerator() const
{
    return m_generator;
}
