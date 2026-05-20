#pragma once

#include <cstdint>

#include "EgoCore/Reference/Pointer.h"

namespace ego::gpu
{
	inline constexpr uint32_t InvalidBindlessIndex = 0xffffffffu;

	class GraphicObject
	{
	public:
		GraphicObject() = default;
		virtual ~GraphicObject() = default;

		virtual void* getNativeHandle() const = 0;

		template<typename T>
		T* getNativeHandle() const { return reinterpret_cast<T*>(getNativeHandle()); }

		virtual void setName(const char* _name) = 0;
	};

	EGO_POINTER(GraphicObject);
}
