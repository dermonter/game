#ifndef WIN32_GAME_H

#include <stdint.h>
#include <windows.h>

#define DEFAULT_WIDTH 968
#define DEFAULT_HEIGHT 540

struct win32_sound_output {
    int samplesPerSecond;
    uint32_t runningSampleIndex;
    int bytesPerSample;
    int secondaryBufferSize;
    int safetyBytes;
};

struct win32_offscreen_buffer {
    HGLRC openGLRenderingContext;
    BITMAPINFO info;
    void* memory;
    int32_t width;
    int32_t height;
    int32_t bytesPerPixel;
    int32_t pitch;
};

struct win32_debug_sound {
    DWORD playCursor;
    DWORD writeCursor;
};

#define WIN32_GAME_H
#endif