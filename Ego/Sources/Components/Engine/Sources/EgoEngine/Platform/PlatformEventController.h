#pragma once

#include "EgoEngine/Platform/Window/Window.h"

namespace ego
{
	class PlatformEventController
	{
	public:
		PlatformEventController();
		virtual ~PlatformEventController();

		virtual void updateNativeEvents() = 0;

	protected:
        static void OnWindowDestroying(WindowPointer _window);
        static void OnWindowActivate(WindowPointer _window, bool _isActive);
        static void OnWindowSizeChange(WindowPointer _window, const WindowSize& _prevSize);

		void initWindowContextEvent();
		void releaseWindowContextEvent();
	};
}
