// handmadehero_4neie.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "handmadehero_4neie.h"
#include "windows.h"

LRESULT CALLBACK MainWindowCallback(
    HWND Window, 
    UINT Message, 
    WPARAM wParam, 
    LPARAM lParam
){
    LRESULT Result = 0;

    switch (Message) {
        case WM_SIZE: 
        {
            OutputDebugStringA("WM_SIZE\n");
        }break;
    
        case WM_DESTROY: 
        {
            OutputDebugStringA("WM_DESTROY\n");
        }break;

        case WM_CLOSE: 
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_CLOSE\n");
        }break;
    
        case WM_ACTIVATEAPP: 
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, Operation);
            Operation = Operation == WHITENESS ? BLACKNESS : WHITENESS;
            EndPaint(Window, &Paint);
        }break;
    
        default: 
        {
            //OutputDebugStringA("Default\n");
            Result = DefWindowProc(Window, Message, wParam, lParam);
        }break;
    }

    return Result;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass)) {
        HWND WindowHandle = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            L"Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            hInstance,
            0
        );

        if (WindowHandle) {
            // Loop
            MSG Message;
            for (;;) {
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if (MessageResult > 0) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else {
                    break;
                }
            }
        }
        else {
            // TODO(Aniket): Logging
        }
    }
    else {
        // TODO(Aniket): Logging
    }

    return 0;
}