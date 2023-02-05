// handmadehero_4neie.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "handmadehero_4neie.h"
#include "windows.h"
#include <stdint.h>

// typedefs
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int32_t int32;


//defines
#define local_persist       static
#define global_variable     static
#define internal            static


// TODO(Aniket): This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;


internal void RenderWeirdGradient(int XOffset, int YOffset) {
    int Width = BitmapWidth;
    int Pitch = Width * BytesPerPixel;
    uint8* Row = (uint8*)BitmapMemory;                  // we want to move byte by byte
    for (int Y = 0; Y < BitmapHeight; ++Y) {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < BitmapWidth; ++X) {
            // set the RGB values
            /*
            * Byte Position:    0  1  2  3
            * Pixel in memory: BB GG RR XX              // Little Endian format - 0x xxBBGGRR - Most significant byte in lower memory
            */

            //THIS IS BLUE
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);
            uint8 Red = (X + Y + YOffset * XOffset);
            *Pixel++ = ((Green << 8) | Blue | (Red << 16));
        }
        Row += Pitch;
    }
}

// Device Independent Bitmap - DIB: This function resizes / initializes the bitmap whenever we get VM_SIZE message
internal void Win32ResizeDIBSection(int Width, int Height) {
    // TODO(Aniket): Maybe dont free first and free the memory after we get the new one, and if getting new one fails then we continue using the old one
    
    if (BitmapMemory) {
        VirtualFree(
            BitmapMemory,
            0,
            MEM_RELEASE
        );
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;  // top-down format for back buffer
    BitmapInfo.bmiHeader.biPlanes = 1;              // recommended
    BitmapInfo.bmiHeader.biBitCount = 32;           // 32 bits because we want double word alignment R, G, B = 24 bits    
    BitmapInfo.bmiHeader.biCompression = BI_RGB;    // no compression
    BitmapInfo.bmiHeader.biSizeImage = 0;           // cause we dont have a compression format
    BitmapInfo.bmiHeader.biXPelsPerMeter = 0;       // this if we don't have a print target
    BitmapInfo.bmiHeader.biYPelsPerMeter = 0;       // this if we don't have a print target
    BitmapInfo.bmiHeader.biClrUsed = 0;
    BitmapInfo.bmiHeader.biClrImportant = 0;

    // NOTE(Aniket): If we are using StretchDIBits
    int BitmapMemorySize = Width * Height * BytesPerPixel;
    BitmapMemory = VirtualAlloc(
        0,                                      // this is set to NULL(0), i.e. system determines where to allocate the region
        BitmapMemorySize,                         // size of the memory required by the back buffer
        MEM_COMMIT,
        PAGE_READWRITE                  
    );   // 4bytes = 32 bits per pixel for the pixels to lie on double word(here 32 bits) boundary.
    //RenderWeirdGradient(128, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height) {
    
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    StretchDIBits(
        DeviceContext,
        /*
        X, Y, Width, Height,                   // this is the origin coordinates, width, and height for the newly sized client
        X, Y, Width, Height,                      // initially we have the both the src and destination rectangle as the same size
        */
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, BitmapWidth, BitmapHeight,
        BitmapMemory,                                                 // the actual bits containing color information that will be blit onto the client's drawing area
        &BitmapInfo,                                                   // pointer to the structure that contains information about the DIB
        DIB_RGB_COLORS,                                               // tells either the bits are RGB or palletized
        SRCCOPY                                                         // what to do to the source rectangle: here we source copy
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

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32UpdateWindow(DeviceContext,&ClientRect,  X, Y, Width, Height);
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
            int XOffset = 0;
            int YOffset = 0;
            // Loop
            while(Running){
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                RenderWeirdGradient(XOffset, YOffset);
                
                HDC DeviceContext = GetDC(WindowHandle);
                RECT ClientRect;
                GetClientRect(WindowHandle, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(
                    DeviceContext,
                    &ClientRect,
                    0,
                    0,
                    WindowWidth,
                    WindowHeight
                );
                ReleaseDC(WindowHandle, DeviceContext);
                XOffset++;
                //YOffset++;
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