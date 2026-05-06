#pragma once

#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
	using EventType = rtti::TypeMetaInfoID;
	constexpr EventType InvalidEventType = 0;

	class Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;

		virtual EventType getType() const = 0;

		EGO_RTTI_VIRTUAL_BASE(Event);
	};
}

#define EGO_EVENT_TYPE_INFO()				\
	static ego::EventType GetEventType()	\
	{										\
		return GetMetaInfoID();				\
	}

#define EGO_EVENT(_TYPE, ...)							\
	EGO_RTTI_VIRTUAL(_TYPE, __VA_ARGS__);				\
	EGO_EVENT_TYPE_INFO();								\
														\
	virtual ego::EventType getType() const override		\
	{													\
		return GetEventType();							\
	}

#define EGO_EVENT_TYPE(_EVENT) (_EVENT::GetEventType())