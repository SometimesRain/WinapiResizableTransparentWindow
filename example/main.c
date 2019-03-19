#include <Windows.h>

#include "window.h"

LRESULT _stdcall WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RegisterWndClass(hInstance, WndProc);
	HWND hwnd = InitInstance(hInstance, "Transparent Window", 800, 600, nCmdShow);

	//Standard Windows message loop
	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return msg.wParam;
}

//Standard window proc function
LRESULT _stdcall WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			//Draw something

			EndPaint(hwnd, &ps);
		}
		break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
	}
	//Call DefTransparentWndProcA instead of DefWindowProcA for default behavior
	return DefTransparentWndProcA(hwnd, message, wParam, lParam);
}