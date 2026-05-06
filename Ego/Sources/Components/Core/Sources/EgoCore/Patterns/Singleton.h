#pragma once

#include "NonCopyable.h"

namespace ego
{
	template <typename T>
	class Singleton : public NonCopyable
	{
	protected:
		Singleton() = default;

	public:
		virtual ~Singleton() = default;

		static T& GetInstance();
	};

	template<typename T>
	inline T& Singleton<T>::GetInstance()
	{
		static T uniqueInstance;
		return uniqueInstance;
	}
}