#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego
{
    class AssertGenerator
    {
    public:
        AssertGenerator() = default;
        virtual ~AssertGenerator() = default;

        virtual void generateError(const char* _message, const char* _file, uint32_t _line) = 0;
    };

    EGO_POINTER(AssertGenerator);

    class AssertController final : public NonCopyable
    {
    public:
        AssertController() = default;
        ~AssertController() override = default;

        bool setGenerator(const AssertGeneratorPointer& _generator);
        void resetGenerator(const AssertGeneratorPointer& _generator = nullptr);

        AssertGeneratorPointer getGenerator() const;

        bool generateError(const char* _message, const char* _file, uint32_t _line) const;

    private:
        mutable std::shared_mutex m_lock;
        AssertGeneratorPointer m_generator = nullptr;
    };

    EGO_POINTER(AssertController);
} // namespace ego
