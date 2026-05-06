#pragma once

#include "EgoCore/UtilsMacros.h"
#include "EgoCore/Patterns/Singleton.h"

#include "AssertGenerator.h"

#include <cassert>

namespace ego
{
	class AssertCore final : public Singleton<AssertCore>
	{
	public:
		AssertCore() = default;

		void setGenerator(AssertGenerator* _generator);
		AssertGenerator* getGenerator() const;

	private:
		AssertGenerator* m_generator = nullptr;
	};

	inline AssertGenerator* GetAssertGenerator() { return AssertCore::GetInstance().getGenerator(); }
}

#if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE)
#define EGO_ASSERT(_CONDITION)														\
	if (!(_CONDITION)) {															\
		ego::AssertGenerator* generator = ego::GetAssertGenerator();				\
		if (generator)																\
		{																			\
			generator->generateError(EGO_TO_STRING(_CONDITION), EGO_FILE, EGO_LINE);\
		}																			\
		else																		\
		{																			\
			assert(_CONDITION);														\
		}																			\
	}


#define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE)									\
	if (!(_CONDITION)) {															\
		ego::AssertGenerator* generator = ego::GetAssertGenerator();				\
		if (generator)																\
		{																			\
			generator->generateError(_MESSAGE, EGO_FILE, EGO_LINE);					\
		}																			\
		else																		\
		{																			\
			assert((_CONDITION) && (_MESSAGE));										\
		}																			\
	}																				
																					
#define EGO_ASSERT_FAIL()															\
	ego::AssertGenerator* generator = ego::GetAssertGenerator();					\
	if (generator)																	\
	{																				\
		generator->generateError("FAIL", EGO_FILE, EGO_LINE);						\
	}																				\
	else																			\
	{																				\
		assert(false);																\
	}																				
																					
#define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE)											\
	ego::AssertGenerator* generator = ego::GetAssertGenerator();					\
	if (generator)																	\
	{																				\
		generator->generateError(_MESSAGE, EGO_FILE, EGO_LINE);						\
	}																				\
	else																			\
	{																				\
		assert(false && (_MESSAGE));												\
	}

#else
#define EGO_ASSERT(_CONDITION) ((void)0)
#define EGO_ASSERT_MESSAGE(_CONDITION, _MESSAGE) ((void)0)
#define EGO_ASSERT_FAIL() ((void)0)
#define EGO_ASSERT_FAIL_MESSAGE(_MESSAGE) ((void)0)
#endif