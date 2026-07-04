#pragma once

#include <cstdint>

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
} // namespace ego
