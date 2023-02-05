// handmadehero_4neie.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "handmadehero_4neie.h"
#include "windows.h"

// typedefs
#define local_persist       static
#define global_variable     static
#define internal            static


// TODO(Aniket): This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

// Device Independent Bitmap - DIB: This function resizes / initializes the bitmap whenever we get VM_SIZE message
internal void Win32ResizeDIBSection(int Width, int Height) {
    // TODO(Aniket): Maybe dont free first and free the memory after we get the new one, and if getting new one fails then we continue using the old one


    if (BitmapHandle) {
        DeleteObject(BitmapHandle);             // Delete the DIBObject
    }
    if (!BitmapDeviceContext) {
        // We need a device context
        //TODO(aniket): Should we recreate this under special circumstances
        BitmapDeviceContext = CreateCompatibleDC(0);  // Passing 0(NULL) will create a DeviceContext compatible with the application's current screen
    }
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;              // recommended
    BitmapInfo.bmiHeader.biBitCount = 32;           // 32 bits because we want double word alignment R, G, B = 24 bits    
    BitmapInfo.bmiHeader.biCompression = BI_RGB;    // no compression
    BitmapInfo.bmiHeader.biSizeImage = 0;           // cause we dont have a compression format
    BitmapInfo.bmiHeader.biXPelsPerMeter = 0;       // this if we don't have a print target
    BitmapInfo.bmiHeader.biYPelsPerMeter = 0;       // this if we don't have a print target
    BitmapInfo.bmiHeader.biClrUsed = 0;
    BitmapInfo.bmiHeader.biClrImportant = 0;

    // create a new DIBSection 
    BitmapHandle = CreateDIBSection(
        BitmapDeviceContext,
        &BitmapInfo,                                    // our bitmap memory is allocated here
        DIB_RGB_COLORS,
        &BitmapMemory,
        0,                                              // NULL
        0                                               // NULL
    );
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height) {
    StretchDIBits(
        DeviceContext,
        X, Y, Width, Height,                             // this is the origin coordinates, width, and height for the newly sized client
        X, Y, Width, Height,                             // initially we have the both the src and destination rectangle as the same size
        BitmapMemory,                             // the actual bits containing color information that will be blit onto the client's drawing area
        &BitmapInfo,                        // pointer to the structure that contains information about the DIB
        DIB_RGB_COLORS,                                  // tells either the bits are RGB or palletized
        SRCCOPY                                          // what to do to the source rectangle: here we source copy
    );
}

LRESULT CALLBACK MainWindowCallback(
    HWND Window, 
    UINT Message, 
    WPARAM wParam, 
    LPARAM lParam
){
    LRESULT Result = 0;             // default response

    switch (Message) {
        case WM_SIZE: 
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);                 // We get the client top, bottom, left, right for the client area.
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        }break;
    
        case WM_DESTROY: 
        {
            // TODO(Aniket): Handle this as an error - recreate window
            Running = false;
        }break;

        case WM_CLOSE: 
        {
            // TODO(Aniket): Handle this with a message to the user?
            Running = false;
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
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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
            Running = true;
            // Loop
            MSG Message;
            while(Running){
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