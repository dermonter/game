#include <windows.h>
#include <stdint.h>

#define global_variable static
#define local_persist static
#define internal static

struct win32_offscreen_buffer {
    BITMAPINFO info;
    void* memory;
    int width;
    int height;
    int bytesPerPixel;
    int pitch;
};

global_variable win32_offscreen_buffer globalBackBuffer;
global_variable bool running;

internal void Win32ResizeDIBSection(win32_offscreen_buffer* buffer, int width, int height) {
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = width * height * buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * buffer->bytesPerPixel;

    uint8_t* row = (uint8_t*)buffer->memory;
    for (int y = 0; y < buffer->height; ++y) {
        uint32_t* pixel = (uint32_t*)row;
        for (int x = 0; x < buffer->width; ++x) {
            uint8_t g = (uint8_t)x;
            uint8_t b = buffer->height - (uint8_t)y;
            // xx RR GG BB
            *pixel++ = (g << 16) | b;
        }
        row += buffer->pitch;
    }
}

internal void Win32UpdateWindow(win32_offscreen_buffer* buffer, HDC deviceContext, RECT windowRect) {
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    StretchDIBits(deviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->width, buffer->height,
        buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message)
    {
    case WM_SIZE: {
    } break;
    case WM_DESTROY:
    case WM_CLOSE: {
        running = false;
    } break;
    case WM_ACTIVATEAPP: {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        RECT clientRect;
        GetClientRect(window, &clientRect);
        HDC deviceContext = BeginPaint(window, &paint);
        Win32UpdateWindow(&globalBackBuffer, deviceContext, clientRect);
        EndPaint(window, &paint);
    } break;
    default: {
        result = DefWindowProc(window, message, wParam, lParam);
    } break;
    }

    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    // HICON hIcon;
    WindowClass.lpszClassName = "Fap Master";

    if (RegisterClassA(&WindowClass)) {
        HWND windowHandle = CreateWindowExA(0,
            WindowClass.lpszClassName,
            "Fap Master",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, hInstance, 0);
        if (windowHandle) {
            running = true;
            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            Win32ResizeDIBSection(&globalBackBuffer, width, height);
            while (running) {
                MSG message;
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                HDC deviceContext = GetDC(windowHandle);
                RECT clientRect;
                GetClientRect(windowHandle, &clientRect);
                Win32UpdateWindow(&globalBackBuffer, deviceContext, clientRect);
                ReleaseDC(windowHandle, deviceContext);
            }
        }
    }

    return 0;
}