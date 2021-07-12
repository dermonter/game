#ifndef WIN32_GAME_H

#include <stdint.h>
#include <windows.h>

struct win32_sound_output {
    int samplesPerSecond;
    uint32_t runningSampleIndex;
    int bytesPerSample;
    int secondaryBufferSize;
    int safetyBytes;
};

struct win32_offscreen_buffer {
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