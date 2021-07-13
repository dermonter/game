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

#include "math.cpp"

struct game_memory {
    bool32_t initialized;
    uint64_t permanentStorageSize;
    void* permanentStorage;
    uint64_t transientStorageSize;
    void* transientStorage;
};

struct game_input {
    real32_t x;
    real32_t y;
};

struct game_state {
    real32_t tSine;
    int32_t playerX;
    int32_t playerY;
    int32_t playerSpeed;
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
internal void GameUpdateAndRender(game_memory* memory, game_offscreen_buffer* buffer, game_input* input);
internal void GameGetSoundSamples(game_memory* memory, game_sound_buffer_output* soundBuffer);

/*
Services the platform layer provides to the game
*/
struct read_file_result {
    void* content;
    uint32_t size;
};
read_file_result PlatformReadEntireFile(const char* filename);
void PlatformFreeEntireFile(void* memory);
bool32_t PlatformWriteEntireFile(char* filename, uint32_t memorySize, void* memory);

#define GAME_H
#endif