#ifndef GAME_H

#if DEBUG
#define assert(expr) if (!(expr)) {*(int*)0 = 0;}
#else
#define assert(expr)
#endif

#define KILO_BYTES(value) ((value) * 1024)
#define MEGA_BYTES(value) (KILO_BYTES(value) * 1024)
#define GIGA_BYTES(value) (MEGA_BYTES(value) * 1024)
#define TERA_BYTES(value) (MEGA_BYTES(value) * 1024)

#define arraySize(array) (sizeof(array) / sizeof((array)[0]))

#define global_variable static
#define local_persist static
#define internal static

typedef double real64_t;
typedef float real32_t;
typedef int bool32_t;
#define PI32 3.14159265359f
#include <stdint.h>
#include <math.h>

struct game_memory {
    bool32_t initialized;
    uint64_t permanentStorageSize;
    void* permanentStorage;
    uint64_t transientStorageSize;
    void* transientStorage;
};

struct game_state {
    real32_t tSine;
};

struct game_offscreen_buffer {
    void* memory;
    int32_t width;
    int32_t height;
    int32_t pitch;
};

struct game_sound_buffer_output {
    int16_t* samples;
    int sampleCount;
    int samplesPerSecond;
};

inline uint32_t SafeUIntTrucate(uint64_t val) {
    assert(fileSize.QuadPart <= 0xffffffff)
    return (uint32_t)val;
}


/*
Services the game provides to the platform layer
*/
void GameUpdateAndRender(game_memory* memory, game_offscreen_buffer* buffer, game_sound_buffer_output* soundBuffer);

/*
Services the platform layer provides to the game
*/
void* PlatformReadEntireFile(char* filename);
void PlatformFreeEntireFile(void* memory);
bool32_t PlatformWriteEntireFile(char* filename, uint32_t memorySize, void* memory);

#define GAME_H
#endif