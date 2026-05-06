#pragma once

#include "EgoCore/PlatformMacros.h"

#include "EgoEngine/Platform/PlatformEventController.h"

namespace ego {
	class Window;
}

namespace ego::win32
{
	class Win32WindowEventController final : public PlatformEventController
	{
	public:
		Win32WindowEventController() = default;

		virtual void updateNativeEvents() override;

		static WNDPROC GetWndProcPtr() { return WndProc; }

	private:
		static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);
		static void UpdateNativeWindowEvents(HWND _hwnd);
	};
}