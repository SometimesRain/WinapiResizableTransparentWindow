## Properly resizable transparent window in pure GDI

Creating a transparent window in GDI has two major challenges to overcome:
* By default, the resize handles are messed up. Dragging the right or bottom border is impossible.
* Window caption starts ignoring mouse input when the window is resized

This small sample program solves these issues.

![](/screenshot.png)

### Usage

```C
#include "window.h"

//Standard Windows window procedure
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
```

### How was it achieved?

Two windows are created, a parent and a child. The parent window contains a see-through border white border with an alpha value of one. The child window is a bog-standard WS_OVERLAPPEDWINDOW with color key transparency. With the parent-child hierarchy, everything just works — the child window correctly processes WM_NCHITTEST messages without any additional code.

**The resize border is almost invisible to human eye**  
![](/against_black_bg.png)
