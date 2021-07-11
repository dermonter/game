#include "game.hpp"
#include "game.cpp"

#include <stdio.h>
#include <windows.h>
#include <dsound.h>

#include "win32_game.hpp"

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#define ASSERT(expr) {if (!expr) *0;}

global_variable LPDIRECTSOUNDBUFFER globalSecondaryBuffer;

bool soundIsPlaying = false;
internal void Win32InitSound(HWND window, uint32_t samplesPerSecond, uint32_t bufferSize) {
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
        if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &globalSecondaryBuffer, 0))) {
            OutputDebugStringA("Secondary buffer was created\n");
        } else {
            // TODO: ERROR CALL
        }
    } else {
        // TODO: ERROR CALL
    }
}

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
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    buffer->pitch = width * buffer->bytesPerPixel;
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
        uint32_t vkCode = (uint32_t)wParam;
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

internal void Win32FillSoundBuffer(win32_sound_output* soundOutput, DWORD bytesToLock, DWORD bytesToWrite, game_sound_buffer_output* soundBuffer) {
    VOID* region1;
    DWORD region1Size;
    VOID* region2;
    DWORD region2Size;
    if (SUCCEEDED(globalSecondaryBuffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0))) {
        DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
        DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
        int16_t* sampleOut = (int16_t*)region1;
        int16_t* sampleIn = (int16_t*)soundBuffer->samples;

        for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
            *sampleOut++ = *sampleIn++;
            *sampleOut++ = *sampleIn++;
            ++soundOutput->runningSampleIndex;
        }

        sampleOut = (int16_t*)region2;
        for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
            *sampleOut++ = *sampleIn++;
            *sampleOut++ = *sampleIn++;
            ++soundOutput->runningSampleIndex;
        }
        globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
    }
}

internal void Win32ClearSoundBuffer(win32_sound_output* soundOutput) {
    VOID* region1;
    DWORD region1Size;
    VOID* region2;
    DWORD region2Size;
    if (SUCCEEDED(globalSecondaryBuffer->Lock(0, soundOutput->secondaryBufferSize, &region1, &region1Size, &region2, &region2Size, 0))) {
        int8_t* sampleOut = (int8_t*)region1;
        for (DWORD sampleIndex = 0; sampleIndex < region1Size; ++sampleIndex) {
            *sampleOut++ = 0;
        }

        sampleOut = (int8_t*)region2;
        for (DWORD sampleIndex = 0; sampleIndex < region2Size; ++sampleIndex) {
            *sampleOut++ = 0;
        }
        globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
    }
}

void* PlatformReadEntireFile(char* filename) {
    void* result = 0;
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file, &fileSize)) {
            assert(fileSize.QuadPart <= 0xffffffff)
            result = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result) {
                DWORD bytesRead;
                uint32_t sizeTruc = SafeUIntTrucate(fileSize.QuadPart);
                if (ReadFile(file, result, sizeTruc, &bytesRead, 0) && (bytesRead == sizeTruc)) {
                    // SUCCESS
                } else {
                    PlatformFreeEntireFile(result);
                }
            }
        }

        CloseHandle(file);
    }
    return result;
}

void PlatformFreeEntireFile(void* memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}

bool32_t PlatformWriteEntireFile(char* filename, uint32_t memorySize, void* memory) {
    bool result = false;
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file) {
        DWORD bytesWritten;
        result = WriteFile(file, memory, memorySize, &bytesWritten, 0);

        CloseHandle(file);
    }
    return result;
}

global_variable int64_t globalPerfFreq;

inline real32_t Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    real32_t result = (real32_t)(end.QuadPart - start.QuadPart) / (real32_t)globalPerfFreq;
    return result;
}

inline LARGE_INTEGER Win32GetWallClock() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline void Win32DisplayBufferInWindow(HWND windowHandle, win32_offscreen_buffer* backBuffer) {
    HDC deviceContext = GetDC(windowHandle);
    RECT cRect;
    GetClientRect(windowHandle, &cRect);
    Win32UpdateWindow(backBuffer, deviceContext, cRect);
    ReleaseDC(windowHandle, deviceContext);
}

internal void Win32DebugDrawVertical(win32_offscreen_buffer* backBuffer, int x, int top, int bottom, uint32_t color) {
    uint8_t* pixel = (uint8_t*)backBuffer->memory + x * backBuffer->bytesPerPixel + top * backBuffer->pitch;
    for (int y = top; y < bottom; ++y) {
        *(uint32_t*)pixel = color;
        pixel += backBuffer->pitch;
    }
}

internal void Win32DebugDisplayDebugSound(win32_offscreen_buffer* backBuffer,
                                    win32_debug_sound* playCursors, int lastPlayCursorCount,
                                    win32_sound_output* soundOutput, real32_t targetSecondsPerFrame) {
    int padding = 16;
    int top = padding;
    int bottom = backBuffer->height - padding;
    real32_t C = (real32_t)(backBuffer->width - 2 * 16) / (real32_t)soundOutput->secondaryBufferSize;
    for (int playCursorI = 0; playCursorI < lastPlayCursorCount; ++playCursorI) {
        win32_debug_sound thisCursors = playCursors[playCursorI];
        int xPlay = padding + (int)(C * (real32_t)thisCursors.playCursor);
        int xWrite = padding + (int)(C * (real32_t)thisCursors.writeCursor);
        Win32DebugDrawVertical(backBuffer, xPlay, top, bottom, 0xFFFFFFFF);
        Win32DebugDrawVertical(backBuffer, xWrite, top, bottom, 0xFFFF0000);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    // Set the Windows scheduler granularity
    UINT desiredSchedulerMs = 1;
    bool32_t sleepIsGranular = timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR;

    LARGE_INTEGER performanceFreq;
    QueryPerformanceFrequency(&performanceFreq);
    globalPerfFreq = performanceFreq.QuadPart;

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    // HICON hIcon;
    WindowClass.lpszClassName = "Fap Master";

#define framesOfAudioLatency 4
#define monitorRefreshHz 144
#define gameUpdateHz 30
    real32_t targetSecondsPerFrame = 1.0f / (real32_t)gameUpdateHz;

    // Handle startup
    DWORD lastPlayCursor = 0;
    bool32_t soundIsValid = false;

#if SLOW
    // TEST CODE
    win32_debug_sound debugPlayCursors[gameUpdateHz / 2] = {0};
    int debugPlayCursorIndex = 0;
#endif

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
            soundOutput.runningSampleIndex = 0;
            soundOutput.bytesPerSample = sizeof(int16_t) * 2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
            soundOutput.latencySampleCount = framesOfAudioLatency * (soundOutput.samplesPerSecond / gameUpdateHz);
            int16_t* samples = (int16_t*)VirtualAlloc(0, soundOutput.secondaryBufferSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

            Win32InitSound(windowHandle, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
            Win32ClearSoundBuffer(&soundOutput);

#if SLOW
            LPVOID baseAddress = (LPVOID)TERA_BYTES((uint64_t)2);
#else
            LPVOID baseAddress = 0;
#endif
            game_memory gameMemory = {};
            gameMemory.transientStorageSize = GIGA_BYTES((uint64_t)1);
            gameMemory.permanentStorageSize = MEGA_BYTES(64);
            uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
            gameMemory.transientStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            gameMemory.permanentStorage = (int8_t*)gameMemory.transientStorage + gameMemory.transientStorageSize;
            if (!gameMemory.permanentStorage || !gameMemory.transientStorage) return 1;

            running = true;
            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            Win32ResizeDIBSection(&globalBackBuffer, width, height);

            LARGE_INTEGER lastCounter = Win32GetWallClock();
            uint64_t lastCycles = __rdtsc();
            while (running) {
                MSG message;
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                DWORD bytesToWrite = 0;
                DWORD bytesToLock = 0;
                if (soundIsValid) {
                    DWORD targetCursor = (lastPlayCursor + soundOutput.bytesPerSample * soundOutput.latencySampleCount) % soundOutput.secondaryBufferSize;
                    bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
                    if (bytesToLock > targetCursor) {
                        bytesToWrite = soundOutput.secondaryBufferSize - bytesToLock;
                        bytesToWrite += targetCursor;
                    } else {
                        bytesToWrite = targetCursor - bytesToLock;
                    }
                }

                game_offscreen_buffer gameBuffer = {};
                gameBuffer.memory = globalBackBuffer.memory;
                gameBuffer.width = globalBackBuffer.width;
                gameBuffer.height = globalBackBuffer.height;
                gameBuffer.pitch = globalBackBuffer.pitch;

                game_sound_buffer_output soundBuffer = {};
                soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
                soundBuffer.sampleCount = bytesToWrite / soundOutput.bytesPerSample;
                soundBuffer.samples = samples;
                GameUpdateAndRender(&gameMemory, &gameBuffer, &soundBuffer);

                if (soundIsValid) {
                    Win32FillSoundBuffer(&soundOutput, bytesToLock, bytesToWrite, &soundBuffer);
                }

                if (!soundIsPlaying) {
                    globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
                    soundIsPlaying = true;
                }
                
                uint64_t endCycles = __rdtsc();
                LARGE_INTEGER workCounter = Win32GetWallClock();
                real32_t workSecondsElapsed = Win32GetSecondsElapsed(lastCounter, workCounter);
                real32_t secondsElapsedForFrame = workSecondsElapsed;

                if (secondsElapsedForFrame < targetSecondsPerFrame) {
                    if (sleepIsGranular) {
                        DWORD sleepMs =  (DWORD)(1000.0f * (targetSecondsPerFrame - secondsElapsedForFrame));

                        if (sleepMs > 0) {
                            Sleep(sleepMs);
                        }
                    }

                    real32_t testSecondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetWallClock());
                    // assert(testSecondsElapsed < targetSecondsPerFrame);
                    while (secondsElapsedForFrame < targetSecondsPerFrame) {
                        secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter, Win32GetWallClock());
                    }
                } else {
                    // MISSED FRAMERATE WE FCKED UP HELP ME GOD PLEASE
                    // TODO: help
                }
                LARGE_INTEGER endCounter = Win32GetWallClock();
                real32_t sPerFrame = Win32GetSecondsElapsed(lastCounter, endCounter);
                real32_t msPerFrame = 1000.0f * sPerFrame;
                lastCounter = endCounter;

#if SLOW
                Win32DebugDisplayDebugSound(&globalBackBuffer, debugPlayCursors, arraySize(debugPlayCursors), &soundOutput, targetSecondsPerFrame);
#endif

                Win32DisplayBufferInWindow(windowHandle, &globalBackBuffer);

                DWORD playCursor;
                DWORD writeCursor;
                if (SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
                    lastPlayCursor = playCursor;
                    if (!soundIsValid) {
                        soundOutput.runningSampleIndex = writeCursor / soundOutput.bytesPerSample;
                        soundIsValid = true;
                    }
                } else {
                    soundIsValid = false;
                }

#if SLOW
                {
                    debugPlayCursors[debugPlayCursorIndex].playCursor = playCursor;
                    debugPlayCursors[debugPlayCursorIndex++].writeCursor = writeCursor;
                    if (debugPlayCursorIndex == arraySize(debugPlayCursors)) {
                        debugPlayCursorIndex = 0;
                    }
                }
#endif
                uint64_t cyclesElapsed = endCycles - lastCycles;
                lastCycles = endCycles;

                real64_t fps = 1 / sPerFrame;
                real64_t mcpf = (real64_t)cyclesElapsed / (1000.0f * 1000.0f);

                char FPSbuffer[256];
                _snprintf_s(FPSbuffer, sizeof(FPSbuffer), "%.02fms/f, %.02fFPS, %.02fMc/f\n", msPerFrame, fps, mcpf);
                OutputDebugStringA(FPSbuffer);
            }
        }
    }

    return 0;
}