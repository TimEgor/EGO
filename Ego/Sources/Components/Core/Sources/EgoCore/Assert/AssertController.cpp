#include "AssertController.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

bool ego::AssertController::setGenerator(const AssertGeneratorPointer& _generator)
{
    EGO_CHECK_RETURN_FALSE(_generator);

    bool isAlreadySet = false;
    {
        std::unique_lock lock(m_lock);
        isAlreadySet = m_generator && m_generator.get() != _generator.get();
        if (!isAlreadySet)
        {
            m_generator = _generator;
        }
    }

    if (isAlreadySet)
    {
        EGO_ASSERT_FAIL_MESSAGE("Assert generator has been already set.");
        return false;
    }

    return true;
}

void ego::AssertController::resetGenerator(const AssertGeneratorPointer& _generator)
{
    AssertGeneratorPointer generator;
    bool hasMismatchedGenerator = false;
    {
        std::unique_lock lock(m_lock);
        hasMismatchedGenerator = _generator && m_generator && m_generator.get() != _generator.get();
        if (!hasMismatchedGenerator)
        {
            generator.swap(m_generator);
        }
    }

    if (hasMismatchedGenerator)
    {
        EGO_ASSERT_FAIL_MESSAGE("Assert generator reset request doesn't match active generator.");
    }
}

ego::AssertGeneratorPointer ego::AssertController::getGenerator() const
{
    std::shared_lock lock(m_lock);
    return m_generator;
}

bool ego::AssertController::generateError(const char* _message, const char* _file, uint32_t _line) const
{
    const AssertGeneratorPointer generator = getGenerator();
    if (!generator)
    {
        return false;
    }

    generator->generateError(_message, _file, _line);
    return true;
}
