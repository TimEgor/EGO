#pragma once

#include "MTCountable.h"

#include <utility>

namespace ego
{
	template <typename T>
	class Reference
	{
		static_assert(std::is_base_of_v<MTCountable, T>);

	public:
		Reference() = default;
		Reference(nullptr_t);
		Reference(T* _object);
		Reference(const Reference& _reference);

		template <typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
		Reference(const Reference<U>& _reference);

		~Reference();

		Reference& operator=(const Reference& _reference);

		T* operator->() const { return m_object; }
		T& operator*() const { return *m_object; }

		operator T*() const { return m_object; }

		bool isNull() const { return m_object == nullptr; }

		T* getObject() const { return m_object; }
		T& getObjectRef() const { return *m_object; }

		template <typename CastType>
		CastType* getObjectCast() const { return static_cast<CastType*>(getObject()); }

		template <typename CastType>
		CastType& getObjectCastRef() const { return static_cast<CastType&>(getObjectRef()); }

		void reset();

	protected:
		void release();
		void assign(T* _object);

		T* m_object = nullptr;
	};
}

#define EGO_REFERENCE_DECLARATION(_TYPE, _NAME, _POSTFIX)	\
	using _NAME##_POSTFIX = ego::Reference<_TYPE>;

#define EGO_REFERENCE(_TYPE)	\
	EGO_REFERENCE_DECLARATION(_TYPE, _TYPE, Reference)

#define EGO_NAMED_REFERENCE(_TYPE, _NAME)	\
	EGO_REFERENCE_DECLARATION(_TYPE, _NAME, Reference)

#include "Reference.hpp"