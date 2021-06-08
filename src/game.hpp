#ifndef GAME_H

#define global_variable static
#define local_persist static
#define internal static

typedef double real64_t;
typedef float real32_t;
typedef int bool32_t;
#define PI32 3.14159265359f
#include <stdint.h>
#include <math.h>

/*
Services the game provides to the platform layer
*/

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

void GameUpdateAndRender(game_offscreen_buffer* buffer, game_sound_buffer_output* soundBuffer);

/*
Services the paltform layer provides to the game
*/

#define GAME_H
#endif