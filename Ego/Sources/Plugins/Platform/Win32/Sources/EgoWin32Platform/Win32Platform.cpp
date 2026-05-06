#include "Win32Platform.h"

#include "Window/Win32Window.h"

ego::win32::Win32Platform::Win32Platform(HINSTANCE _instance)
    : m_instance(_instance)
{}

bool ego::win32::Win32Platform::init()
{
	EGO_CHECK_INITIALIZATION(initWindowClass());

    m_mainWindowProvider = new Win32MainWindowProvider();
    EGO_CHECK_INITIALIZATION(m_mainWindowProvider);

	m_platformEventController = new Win32WindowEventController();
	EGO_CHECK_INITIALIZATION(m_platformEventController);

    return true;
}

void ego::win32::Win32Platform::release()
{
	EGO_SAFE_DESTROY(m_platformEventController);
	EGO_SAFE_DESTROY(m_mainWindowProvider);
}

ego::MainWindowProvider& ego::win32::Win32Platform::getMainWindowProvider()
{
	EGO_ASSERT(m_mainWindowProvider);
    return *m_mainWindowProvider;
}

const ego::MainWindowProvider& ego::win32::Win32Platform::getMainWindowProvider() const
{
	EGO_ASSERT(m_mainWindowProvider);
	return *m_mainWindowProvider;
}

ego::WindowPointer ego::win32::Win32Platform::createWindow(const char* _title, const WindowSize& _size)
{
	WindowPointer window = Win32WindowPointer(new Win32Window());
	window->init(_title, _size);

    return window;
}

const ego::PlatformEventController& ego::win32::Win32Platform::getPlatformEventController() const
{
	EGO_ASSERT(m_platformEventController);
	return *m_platformEventController;
}

ego::PlatformEventController& ego::win32::Win32Platform::getPlatformEventController()
{
	EGO_ASSERT(m_platformEventController);
	return *m_platformEventController;
}

HINSTANCE ego::win32::Win32Platform::getInstanceHandle() const
{
    return m_instance;
}

bool ego::win32::Win32Platform::initWindowClass()
{
	WNDCLASS wndClass{};
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = sizeof(Win32WindowWeakPointer*);
	wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClass.hInstance = m_instance;
	wndClass.lpfnWndProc = Win32WindowEventController::GetWndProcPtr();
	wndClass.lpszClassName = EGO_WIN32_WND_CLASS_NAME;
	wndClass.lpszMenuName = 0;
	wndClass.style = 0;

	if (!RegisterClass(&wndClass))
	{
		EGO_ASSERT_FAIL_MESSAGE("WNDCLASS registration has been failed.");
		return false;
	}

	return true;
}
