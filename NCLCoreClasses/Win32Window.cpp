#include "Win32Window.h"
#ifdef _WIN32
#include "Windowsx.h"
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
using namespace NCL;
using namespace Win32Code;

#define WINDOWCLASS "WindowClass"

Win32Window* Win32Window::instance = nullptr;

Win32Window::Win32Window(const WindowInitialisation& winInitInfo) {
	instance = this;
	forceQuit		= false;
	init			= false;
	mouseLeftWindow	= false;
	lockMouse		= false;
	showMouse		= true;
	active			= true;

	windowTitle = winInitInfo.windowTitle;
	fullScreen	= winInitInfo.fullScreen;

	size = Vector2i(winInitInfo.width, winInitInfo.height);
	defaultSize = size;

	windowInstance = GetModuleHandle(NULL);

	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

	if (!GetClassInfoEx(windowInstance, WINDOWCLASS, &windowClass))	{
		windowClass.cbSize		= sizeof(WNDCLASSEX);
	    windowClass.style		= CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc	= (WNDPROC)WindowProc;
		windowClass.hInstance	= windowInstance;
		windowClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		windowClass.lpszClassName = WINDOWCLASS;

		if(!RegisterClassEx(&windowClass)) {
			std::cout << __FUNCTION__ << " Failed to register class!\n";
			return;
		}
	}

	if(fullScreen == FullScreenState::Exclusive) {
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared

		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth		= winInitInfo.width;// Selected Screen Width
		dmScreenSettings.dmPelsHeight		= winInitInfo.height;// Selected Screen Height
		dmScreenSettings.dmBitsPerPel		= 32;				// Selected Bits Per Pixel
		dmScreenSettings.dmDisplayFrequency = winInitInfo.refreshRate;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)	{
			std::cout << __FUNCTION__ << " Failed to switch to fullscreen!\n";
			return;
		}
	}

	int exStyleFlags = 0;
	int styleFlags = WS_POPUP | WS_VISIBLE;
	switch (fullScreen)
	{
	case FullScreenState::None:
		styleFlags |= WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
		break;
	case FullScreenState::Exclusive:
		// This would prevent us from alt-tabbing
		//exStyleFlags |= WS_EX_TOPMOST; // Always on top
		break;
	case FullScreenState::Borderless:
		break;
	default: assert(false);
	}

	windowHandle = CreateWindowEx(exStyleFlags,
		WINDOWCLASS,							// name of the window class
		winInitInfo.windowTitle.c_str(),		// title of the window
		styleFlags,    // window style
		CW_USEDEFAULT,			// x-position of the window
		CW_USEDEFAULT,			// y-position of the window
		winInitInfo.width,				// width of the window
		winInitInfo.height,				// height of the window
        NULL,				// No parent window!
        NULL,				// No Menus!
		windowInstance,		// application handle
        NULL);				// No multiple windows!

 	if(!windowHandle) {
		std::cout << __FUNCTION__ << " Failed to create window!\n";
		return;
	}

	winMouse	= new Win32Mouse(windowHandle);
	winKeyboard = new Win32Keyboard(windowHandle);

	keyboard	= winKeyboard;
	mouse		= winMouse;
	winMouse->SetAbsolutePositionBounds(size);

	winMouse->Wake();
	winKeyboard->Wake();

	LockMouseToWindow(lockMouse);
	ShowOSPointer(showMouse);

	init		= true;
	maximised	= false;
	minimised	= false;
}

Win32Window::~Win32Window(void)	{
	instance = nullptr;
	init = false;
}

bool	Win32Window::InternalUpdate() {
	MSG		msg;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(windowHandle, &pt);
	winMouse->SetAbsolutePosition(Vector2((float)pt.x, (float)pt.y));

	while(PeekMessage(&msg,windowHandle,0,0,PM_REMOVE)) {
		CheckMessages(msg);
	}

	return !forceQuit;
}

void	Win32Window::UpdateTitle()	{
	SetWindowText(windowHandle, windowTitle.c_str());
}

void	Win32Window::SetFullScreen(bool fullScreen) {
	if (fullScreen) {
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared

		DEVMODEA settings;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &settings);

		size.x = (float)settings.dmPelsWidth;
		size.y = (float)settings.dmPelsHeight;

		dmScreenSettings.dmSize				= sizeof(dmScreenSettings);			// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth		= (DWORD)size.x;		// Selected Screen Width
		dmScreenSettings.dmPelsHeight		= (DWORD)size.y;		// Selected Screen Height
		dmScreenSettings.dmBitsPerPel		= 32;								// Selected Bits Per Pixel
		dmScreenSettings.dmDisplayFrequency = (DWORD)settings.dmDisplayFrequency;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			std::cout << __FUNCTION__ << " Failed to switch to fullscreen!\n";
		}
		else {
			if (eventHandler) {
				eventHandler(fullScreen ? NCL::WindowEvent::Fullscreen : NCL::WindowEvent::Windowed, size.x, size.y);
			}
		}
	}
	else {
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared

		size = defaultSize;

		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth  = (DWORD)size.x;		// Selected Screen Width
		dmScreenSettings.dmPelsHeight = (DWORD)size.y;		// Selected Screen Height
		dmScreenSettings.dmPosition.x = (DWORD)position.x;
		dmScreenSettings.dmPosition.y = (DWORD)position.y;
		dmScreenSettings.dmBitsPerPel = 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmDisplayFrequency = 60;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY | DM_POSITION;

		if (ChangeDisplaySettings(&dmScreenSettings, 0) != DISP_CHANGE_SUCCESSFUL) {
			std::cout << __FUNCTION__ << " Failed to switch out of fullscreen!\n";
		}
	}
}

void Win32Window::CheckMessages(MSG &msg) {
	switch (msg.message)	{				// Is There A Message Waiting?
		case (WM_QUIT):
		case (WM_CLOSE): {					// Have We Received A Quit Message?
			instance->ShowOSPointer(true);
			instance->LockMouseToWindow(false);
			forceQuit = true;
		}break;
		case (WM_INPUT): {
			UINT dwSize;
			GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize,sizeof(RAWINPUTHEADER));

			BYTE* lpb = new BYTE[dwSize];

			GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize,sizeof(RAWINPUTHEADER));
			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (keyboard && raw->header.dwType == RIM_TYPEKEYBOARD && active) {
				instance->winKeyboard->UpdateRAW(raw);
			}

			if (mouse && raw->header.dwType == RIM_TYPEMOUSE && active) {
				instance->winMouse->UpdateRAW(raw);
			}

			delete lpb;
		}break;

		default: {								// If Not, Deal With Window Messages
			TranslateMessage(&msg);				// Translate The Message
			DispatchMessage(&msg);				// Dispatch The Message
		}
	}
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)	{
	bool applyResize = false;

    switch(message)	 {
        case(WM_DESTROY):	{
			instance->ShowOSPointer(true);
			instance->LockMouseToWindow(false);

			PostQuitMessage(0);
			instance->forceQuit = true;
		} break;
		case (WM_ACTIVATE): {
			if(LOWORD(wParam) == WA_INACTIVE)	{
				instance->active = false;
				ReleaseCapture();
				ClipCursor(NULL);
				if (instance->init && instance->mouse && instance->keyboard) {
					if (instance->winMouse) {
						instance->winMouse->Sleep();
					}
					if (instance->winKeyboard) {
						instance->winKeyboard->Sleep();
					}
				}
			}
			else {
				instance->active = true;
				if(instance->init) {
					instance->winMouse->Wake();
					instance->winKeyboard->Wake();

					if(instance->lockMouse) {
						instance->LockMouseToWindow(true);
					}
				}
			}
			return 0;
		}break;
		case (WM_SYSCOMMAND): {
			if (wParam == SC_RESTORE) {
				if (instance->minimised) {
					ShowWindow(instance->windowHandle, SW_RESTORE);
					if (instance->init) {
						instance->winMouse->SetAbsolutePositionBounds(instance->size);
						instance->LockMouseToWindow(instance->lockMouse);
					}
				}
			}
		}break;
		case (WM_LBUTTONDOWN): {
			if(instance->init && instance->lockMouse) {
				instance->LockMouseToWindow(true);
			}
		}break;
		case (WM_MOUSEMOVE): {
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = instance->windowHandle;
			TrackMouseEvent(&tme);

			if (instance->winMouse) {
				instance->winMouse->UpdateWindowPosition(
					Vector2i(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
				);
			}

			if(instance->mouseLeftWindow) {
				instance->mouseLeftWindow = false;
				if (instance->init) {
					instance->winMouse->Wake();
					instance->winKeyboard->Wake();
				}
			}
		}break;
		case(WM_MOUSELEAVE):{
			instance->mouseLeftWindow = true;
			if (instance->init) {
				instance->winMouse->Sleep();
				instance->winKeyboard->Sleep();
			}
		}break;
		case(WM_SIZE): {
			float newX = (float)LOWORD(lParam);
			float newY = (float)HIWORD(lParam);
			if (newX > 0 && newY > 0 && (newX != instance->size.x || newY != instance->size.y)) {
				instance->size.x = (float)LOWORD(lParam);
				instance->size.y = (float)HIWORD(lParam);
			}
			if (wParam == SIZE_MINIMIZED) {
				instance->minimised = true;
				//applyResize = true;
			}
			if (wParam == SIZE_MAXIMIZED) {
				instance->minimised = false;
				instance->maximised = true;
				applyResize = true;
			}
			else if (wParam == SIZE_RESTORED && instance->maximised) {
				instance->maximised = false;
				applyResize = true;
			}
			else if (wParam == SIZE_RESTORED && instance->minimised) {
				instance->minimised = false;
				applyResize = true;
			}
		}break;
		case(WM_ENTERSIZEMOVE): {
		}break;
		case(WM_EXITSIZEMOVE): {
			applyResize = true;
		}break;
    }

	if (applyResize) {
		if (instance->eventHandler) {
			instance->eventHandler(NCL::WindowEvent::Resize, instance->size.x, instance->size.y);
		}

		if (instance->init) {
			instance->winMouse->SetAbsolutePositionBounds(instance->size);
			instance->LockMouseToWindow(instance->lockMouse);
		}
	}

    return DefWindowProc (hWnd, message, wParam, lParam);
}

void	Win32Window::LockMouseToWindow(bool lock)	{
	lockMouse = lock;
	if(lock) {
		RECT		windowRect;
		GetWindowRect (windowHandle, &windowRect);

		SetCapture(windowHandle);
		ClipCursor(&windowRect);
	}
	else{
		ReleaseCapture();
		ClipCursor(NULL);
	}
}

void	Win32Window::ShowOSPointer(bool show)	{
	if(show == showMouse) {
		return;	//ShowCursor does weird things, due to being a counter internally...
	}

	showMouse = show;
	if(show) {
		ShowCursor(1);
	}
	else{
		ShowCursor(0);
	}
}

void	Win32Window::SetConsolePosition(int x, int y)	{
	HWND consoleWindow = GetConsoleWindow();

	SetWindowPos(consoleWindow, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SetActiveWindow(windowHandle);
}

void	Win32Window::SetWindowPosition(int x, int y) {
	SetWindowPos(windowHandle, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SetActiveWindow(windowHandle);
}

void	Win32Window::ShowConsole(bool state)				{
	HWND consoleWindow = GetConsoleWindow();

	ShowWindow(consoleWindow, state ? SW_RESTORE : SW_HIDE);

	SetActiveWindow(windowHandle);
}

#endif //_WIN32
