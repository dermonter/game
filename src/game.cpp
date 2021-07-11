#include "game.hpp"

internal void RenderGradient(game_offscreen_buffer* buffer) {
    uint8_t* row = (uint8_t*)buffer->memory;
    for (int y = 0; y < buffer->height; ++y) {
        uint32_t* pixel = (uint32_t*)row;
        for (int x = 0; x < buffer->width; ++x) {
            uint8_t g = (uint8_t)x;
            uint8_t b = (uint8_t)buffer->height - (uint8_t)y;
            // xx RR GG BB
            *pixel++ = (g << 8) | b;
        }
        row += buffer->pitch;
    }
}

internal void GameOutputSound(game_state* gameState, game_sound_buffer_output* buffer) {
    int16_t toneVolume = 1000;
    int toneHz = 256;
    int wavePeriod = buffer->samplesPerSecond / toneHz;

    int16_t* sampleOut = buffer->samples;
    for (int sampleIndex = 0; sampleIndex < buffer->sampleCount; ++sampleIndex) {
        real32_t sinValue = sinf(gameState->tSine);
        int16_t sampleValue = (int16_t)(sinValue * toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        gameState->tSine += 2.0f * PI32 * 1.0f / (real32_t)wavePeriod;
    }
}

void GameUpdateAndRender(game_memory* memory, game_offscreen_buffer* buffer, game_sound_buffer_output* soundBuffer) {
    assert(sizeof(game_state) <= memory->permanentStorageSize);
    game_state* gameState = (game_state*)memory->permanentStorage;

    if (!memory->initialized) {
        memory->initialized = true;
    }

    GameOutputSound(gameState, soundBuffer);

    RenderGradient(buffer);
}