#include "window.h"

#define CLASS_NAME "TransparentWnd"
#define BORDER_COLOR 0x00ffffff

typedef struct
{
	RECT rect;
	RECT border;
	HWND mainWindow;
	HWND borderWindow;

	HDC hWindow;
	HDC hMemory;
	HBITMAP hBitmap;
	BLENDFUNCTION alphaOne;
	UINT* bitmapData;
	int bitmapWidth;
	int bitmapHeight;
} window_t;

void Draw(HWND hwnd, UINT color);
void CleanUp(HWND hwnd);

window_t window;

ATOM RegisterWndClass(HINSTANCE hInstance, WNDPROC wndProc)
{
	WNDCLASSEXA wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEXA));

	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.hInstance = hInstance;
	wcex.hbrBackground = CreateSolidBrush(TRANSPARENCY_KEY);
	wcex.lpszClassName = CLASS_NAME;
	wcex.hCursor = LoadCursorA(NULL, IDC_ARROW);

	return RegisterClassExA(&wcex);
}

HWND InitInstance(HINSTANCE hInstance, const char* title, int width, int height, int nCmdShow)
{
	//Create blend function
	window.alphaOne.BlendOp = AC_SRC_OVER;
	window.alphaOne.SourceConstantAlpha = 1;
	window.alphaOne.AlphaFormat = AC_SRC_ALPHA;
	window.alphaOne.BlendFlags = 0;

	//Get monitor size
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	POINT monitorPos;
	GetCursorPos(&monitorPos);
	GetMonitorInfoA(MonitorFromPoint(monitorPos, MONITOR_DEFAULTTONEAREST), &info);
	window.bitmapWidth = info.rcMonitor.right;
	window.bitmapHeight = info.rcMonitor.bottom;

	//Create outer window that provides proper drag handles for the main window
	window.borderWindow = CreateWindowExA(WS_EX_LAYERED, CLASS_NAME, title, WS_POPUP,
		(window.bitmapWidth - width) / 2, (window.bitmapHeight - height) / 2, width, height, NULL, NULL, hInstance, NULL);

	//Create main window
	window.mainWindow = CreateWindowExA(WS_EX_LAYERED, CLASS_NAME, title, WS_OVERLAPPEDWINDOW,
		(window.bitmapWidth - width) / 2, (window.bitmapHeight - height) / 2, width, height, window.borderWindow, NULL, hInstance, NULL);
	SetLayeredWindowAttributes(window.mainWindow, TRANSPARENCY_KEY, 0, LWA_COLORKEY);

	//Construct bitmap info header
	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biWidth = window.bitmapWidth;
	bi.biHeight = -window.bitmapHeight;

	//Create drawing context
	window.hWindow = GetWindowDC(window.borderWindow);
	window.hMemory = CreateCompatibleDC(window.hWindow);

	//Create memory bitmap
	window.hBitmap = CreateDIBSection(window.hMemory, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &window.bitmapData, NULL, 0);
	memset(window.bitmapData, 0, window.bitmapWidth * window.bitmapHeight * sizeof(UINT));
	SelectObject(window.hMemory, window.hBitmap);

	//Get client and window rects
	RECT client;
	GetWindowRect(window.borderWindow, &window.rect);
	GetClientRect(window.mainWindow, &client);
	MapWindowPoints(window.mainWindow, NULL, (POINT*)&client, 2);

	//calculate borders
	window.border.left = client.left - window.rect.left;
	window.border.top = client.top - window.rect.top;
	window.border.right = window.rect.right - client.right;
	window.border.bottom = window.rect.bottom - client.bottom;

	//Draw border
	Draw(window.borderWindow, BORDER_COLOR);

	//Show windows
	ShowWindow(window.mainWindow, nCmdShow);
	ShowWindow(window.borderWindow, nCmdShow);
	SetForegroundWindow(window.mainWindow);

	//Update
	UpdateWindow(window.borderWindow);
	UpdateWindow(window.mainWindow);

	return window.mainWindow;
}

void Draw(HWND hwnd, UINT color)
{
	POINT zero = { 0, 0 };
	SIZE size = { window.rect.right - window.rect.left, window.rect.bottom - window.rect.top };

	//Bottom border
	for (int x = 0; x < size.cx; x++)
		for (int y = size.cy - window.border.bottom; y < size.cy; y++)
			window.bitmapData[y * window.bitmapWidth + x] = color;

	//Left border
	for (int x = 0; x < window.border.left; x++)
		for (int y = 0; y < size.cy; y++)
			window.bitmapData[y * window.bitmapWidth + x] = color;

	//Right border
	for (int x = size.cx - window.border.right; x < size.cx; x++)
		for (int y = 0; y < size.cy; y++)
			window.bitmapData[y * window.bitmapWidth + x] = color;

	//Create transparency
	UpdateLayeredWindow(hwnd, window.hWindow, (POINT*)&window.rect.left, &size, window.hMemory, &zero, 0, &window.alphaOne, ULW_ALPHA);
}

void CleanUp(HWND hwnd)
{
	DeleteObject(window.hBitmap);
	DeleteDC(window.hMemory);
	ReleaseDC(window.borderWindow, window.hWindow);
}

void ActivateFrame(HWND hwnd)
{
	//Sometimes layered windows start ignoring mouse events
	//but resetting the window style fixes this issue
	LONG style = GetWindowLongA(hwnd, GWL_STYLE);
	SetWindowLongA(hwnd, GWL_STYLE, style & ~(WS_CAPTION | WS_THICKFRAME));
	SetWindowLongA(hwnd, GWL_STYLE, style | WS_CAPTION | WS_THICKFRAME);
}

LRESULT _stdcall DefTransparentWndProcA(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Ignore style change messages
	//Otherwise ActivateFrame might cause flicker
	if (message == WM_STYLECHANGING || message == WM_STYLECHANGED)
	{
		return 0;
	}
	//Hide drag handles when resizing
	else if (message == WM_ENTERSIZEMOVE)
	{
		Draw(window.borderWindow, 0);
	}
	//Update window size and redraw handles after resizing
	else if (message == WM_EXITSIZEMOVE)
	{
		GetWindowRect(window.mainWindow, &window.rect);

		Draw(window.borderWindow, 0x01ffffff);
		ActivateFrame(window.mainWindow);
		return 0;
	}
	else if (message == WM_SYSCOMMAND)
	{
		if (wParam == SC_RESTORE || wParam == SC_RESTORE + 2)
		{
			//Window was unminimized
			if (hwnd == window.borderWindow)
			{
				DefWindowProcA(window.borderWindow, message, wParam, lParam);
				ShowWindow(window.mainWindow, SW_SHOW);

				SetForegroundWindow(window.mainWindow);
				return 0;
			}
			//Window was unmaximized
			else
			{
				Draw(window.borderWindow, BORDER_COLOR);
				ActivateFrame(window.mainWindow);
			}
		}
		//Handle minimization
		else if (wParam == SC_MINIMIZE)
		{
			ShowWindow(window.mainWindow, SW_HIDE);
			DefWindowProcA(window.borderWindow, message, wParam, lParam);
			return 0;
		}
		//Handle maximization
		else if (wParam == SC_MAXIMIZE || wParam == SC_MAXIMIZE + 2)
		{
			Draw(window.borderWindow, 0);
			ActivateFrame(window.mainWindow);
		}
	}
	return DefWindowProcA(hwnd, message, wParam, lParam);
}
