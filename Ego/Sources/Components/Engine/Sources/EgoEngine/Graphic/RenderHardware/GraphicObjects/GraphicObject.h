#pragma once

#include <cstdint>

#include "EgoCore/Reference/Reference.h"

namespace ego::gpu
{
	inline constexpr uint32_t InvalidBindlessIndex = -1;

	class GraphicObject : public STDDestroyMTCountable
	{
	public:
		GraphicObject() = default;
		virtual ~GraphicObject() = default;

		virtual void* getNativeHandle() const = 0;

		template<typename T>
		T* getNativeHandle() const { return reinterpret_cast<T*>(getNativeHandle()); }

		virtual void setName(const char* _name) = 0;
	};

	EGO_REFERENCE(GraphicObject);
}
