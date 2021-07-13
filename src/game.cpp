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

        if (gameState->tSine > 2.0f * PI32) {
            gameState->tSine -= 2.0f * PI32;
        }
    }
}

#pragma pack(push, 1)
struct bmp_header {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
};
#pragma pack(pop)

struct bmp {
    uint32_t* pixels;
    int32_t width;
    int32_t height;
};

internal bmp LoadBMP(const char* filename) {
    read_file_result readResult = PlatformReadEntireFile(filename);
    if (readResult.size == 0) return {};
    bmp_header* header = (bmp_header*)readResult.content;

    uint32_t* pixels = (uint32_t*)((uint8_t*)readResult.content + header->dataOffset);
    return {pixels, header->width, header->height};
}

internal void DrawTexture(game_offscreen_buffer* buffer, bmp* texture, int32_t initial_x, int32_t initial_y) {
    // TODO: handle overflow/underflow
    if (initial_x + texture->width < 0) return;
    if (initial_y + texture->height < 0) return;
    if (initial_x >= buffer->width) return;
    if (initial_y >= buffer->height) return;

    uint32_t* start_penis_pixels = texture->pixels;
    for (int y = 0; y < texture->height; ++y) {
        for (int x = 0; x < texture->width; ++x) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)((uint32_t*)buffer->memory + initial_x + x) + (initial_y - y + texture->height) * buffer->pitch);
            uint32_t* penis_pixels = start_penis_pixels + x + y * texture->width;
            if (initial_x + x < 0) continue;
            if (initial_y + texture->height - y < 0) continue;
            if (initial_x + x >= buffer->width) continue;
            if (initial_y + texture->height - y >= buffer->height) continue;

            vec3f color;
            for (int i = 0; i < 3; ++i) {
                color.values[2 - i] = *((uint8_t*)penis_pixels + i);
            }
            uint8_t a = *((uint8_t*)penis_pixels + 3);

            vec3f old_c;
            for (int i = 0; i < 3; ++i) {
                old_c.values[2 - i] = *((uint8_t*)pixel + i);
            }

            real32_t coeff = (real32_t)a / 255.0f;

            vec3f new_c = interpolate(color, old_c, coeff);

            *pixel++ = ((uint8_t)new_c.x << 16) | ((uint8_t)new_c.y << 8) | (uint8_t)new_c.z;
        }
    }
}

internal void GameUpdateAndRender(game_memory* memory, game_offscreen_buffer* buffer, game_input* input) {
    assert(sizeof(game_state) <= memory->permanentStorageSize);
    game_state* gameState = (game_state*)memory->permanentStorage;
    bmp* penis = (bmp*)(gameState + 1);

    if (!memory->initialized) {
        memory->initialized = true;
        *penis = LoadBMP("penis.bmp");
        gameState->playerX = 200;
        gameState->playerY = 20;
        gameState->playerSpeed = 15;
    }

    gameState->playerX += (int32_t)input->x * gameState->playerSpeed;
    gameState->playerY -= (int32_t)input->y * gameState->playerSpeed;

    RenderGradient(buffer);
    DrawTexture(buffer, penis, gameState->playerX, gameState->playerY);
}

internal void GameGetSoundSamples(game_memory* memory, game_sound_buffer_output* soundBuffer) {
    assert(sizeof(game_state) <= memory->permanentStorageSize);
    game_state* gameState = (game_state*)memory->permanentStorage;

    if (!memory->initialized) {
        memory->initialized = true;
    }

    GameOutputSound(gameState, soundBuffer);
}