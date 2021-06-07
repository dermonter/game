#include "game.hpp"

void GameUpdateAndRender(game_offscreen_buffer* buffer) {
    RenderGradient(buffer);
}

internal void RenderGradient(game_offscreen_buffer* buffer) {
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