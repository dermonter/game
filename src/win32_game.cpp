#include <windows.h>
#include <dsound.h>
#include <stdint.h>
#include <math.h>

#define global_variable static
#define local_persist static
#define internal static

typedef double real64_t;
typedef float real32_t;
#define PI32 3.14159265359f

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#define ASSERT(expr) {if (!expr) *0;}

global_variable LPDIRECTSOUNDBUFFER globalSecondaryBuffer;

struct win32_sound_output {
    int samplesPerSecond;
    int toneVolume;
    int toneHz;
    uint32_t runningSampleIndex;
    int wavePeriod;
    int bytesPerSample;
    int secondaryBufferSize;
};

bool soundIsPlaying = false;
internal void Win32InitSound(HWND window, int32_t samplesPerSecond, int32_t bufferSize) {
    HMODULE soundLibrary = LoadLibraryA("dsound.dll");
    if (!soundLibrary) return;

    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;
    direct_sound_create* directSoundCreate = (direct_sound_create*)GetProcAddress(soundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND directSound;
    if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0))) {
        if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
            DSBUFFERDESC bufferDescription = {sizeof(bufferDescription)};
            bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

            LPDIRECTSOUNDBUFFER primaryBuffer;
            if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0))) {
                if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
                    OutputDebugStringA("Format to Primary Sound buffer was set\n");
                }
            } else {
                // TODO: ERROR CALL
            }
        } else {
            // TODO: ERROR CALL
        }

        DSBUFFERDESC bufferDescription = {};
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = 0;
        bufferDescription.dwBufferBytes = bufferSize;
        bufferDescription.lpwfxFormat = &waveFormat;
        HRESULT r = directSound->CreateSoundBuffer(&bufferDescription, &globalSecondaryBuffer, 0);
        if (SUCCEEDED(r)) {
            OutputDebugStringA("Secondary buffer was created\n");
        } else {
            // TODO: ERROR CALL
        }
    } else {
        // TODO: ERROR CALL
    }
}

struct win32_offscreen_buffer {
    BITMAPINFO info;
    void* memory;
    int32_t width;
    int32_t height;
    int32_t bytesPerPixel;
    int32_t pitch;
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
}

internal void RenderGradient(win32_offscreen_buffer* buffer) {
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

internal LRESULT Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
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
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
        uint32_t vkCode = wParam;
        bool wasDown = ((lParam & (1 << 31)) != 0);
        bool isDown = ((lParam & (1 << 31)) == 0);
        if (wasDown == isDown) break;
        switch (vkCode)
        {
        case 'W':
        case 'A':
        case 'S':
        case 'D': {
            
        } break;
        case VK_ESCAPE: {
            if (isDown) {
                running = false;
            }
        } break;
        default:
            break;
        }
    } break;
    default: {
        result = DefWindowProc(window, message, wParam, lParam);
    } break;
    }

    return result;
}

internal void Win32FillSoundBuffer(win32_sound_output* soundOutput, DWORD bytesToLock, DWORD bytesToWrite) {
    VOID* region1;
    DWORD region1Size;
    VOID* region2;
    DWORD region2Size;
    if (SUCCEEDED(globalSecondaryBuffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0))) {
        DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
        DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
        int16_t* sampleOut = (int16_t*)region1;
        for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
            real32_t t = 2.0f * PI32 * (real32_t)soundOutput->runningSampleIndex / (real32_t)soundOutput->wavePeriod;
            real32_t sinValue = sinf(t);
            int16_t sampleValue = (int16_t)(sinValue * soundOutput->toneVolume);
            *sampleOut++ = sampleValue;
            *sampleOut++ = sampleValue;
            ++soundOutput->runningSampleIndex;
        }

        sampleOut = (int16_t*)region2;
        for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
            real32_t t = 2.0f * PI32 * (real32_t)soundOutput->runningSampleIndex / (real32_t)soundOutput->wavePeriod;
            real32_t sinValue = sinf(t);
            int16_t sampleValue = (int16_t)(sinValue * soundOutput->toneVolume);
            *sampleOut++ = sampleValue;
            *sampleOut++ = sampleValue;
            ++soundOutput->runningSampleIndex;
        }
        globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
    }
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
            // Sound Test
            win32_sound_output soundOutput = {};
            soundOutput.samplesPerSecond = 48000;
            soundOutput.toneVolume = 1000;
            soundOutput.toneHz = 256;
            soundOutput.runningSampleIndex = 0;
            soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
            soundOutput.bytesPerSample = sizeof(int16_t) * 2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;

            Win32InitSound(windowHandle, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
            Win32FillSoundBuffer(&soundOutput, 0, soundOutput.secondaryBufferSize);

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

                RenderGradient(&globalBackBuffer);

                DWORD playCursor;
                DWORD writeCursor;
                if (SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
                    DWORD bytesToWrite;
                    DWORD bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
                    if (bytesToLock == playCursor) {
                        if (soundIsPlaying) {
                            bytesToWrite = 0;
                        } else {
                            bytesToWrite = soundOutput.secondaryBufferSize;
                        }
                    } else if (bytesToLock > playCursor) {
                        bytesToWrite = soundOutput.secondaryBufferSize - bytesToLock;
                        bytesToWrite += playCursor;
                    } else {
                        bytesToWrite = playCursor - bytesToLock;
                    }

                    Win32FillSoundBuffer(&soundOutput, bytesToLock, bytesToWrite);
                }

                if (!soundIsPlaying) {
                    globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
                    soundIsPlaying = true;
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