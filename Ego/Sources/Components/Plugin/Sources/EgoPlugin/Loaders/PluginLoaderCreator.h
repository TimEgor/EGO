#pragma once

#if defined(WIN32) || defined(_WIN32)
#include "Win32\Win32PluginLoader.h"
#endif

namespace ego
{
	inline PluginLoaderReference CreatePluginLoader()
	{
#if defined(WIN32) || defined(_WIN32)
		return new Win32PluginLoader();
#endif
	}
}
