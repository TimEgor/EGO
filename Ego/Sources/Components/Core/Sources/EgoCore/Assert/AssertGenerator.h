#pragma once

#include "EgoCore/Reference/Pointer.h"

#include <cstdint>

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
}
