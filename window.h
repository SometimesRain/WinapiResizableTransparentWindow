#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

#define TRANSPARENCY_KEY 0xfe00fe

ATOM RegisterWndClass(HINSTANCE hInstance, WNDPROC wndProc);
HWND InitInstance(HINSTANCE hInstance, const char* title, int width, int height, int nCmdShow);
LRESULT _stdcall DefTransparentWndProcA(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif //WINDOW_H