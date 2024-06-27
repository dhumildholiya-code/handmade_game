#include "game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gl_renderer.cpp"

inline internal real32 Sign(real32 x)
{
    if(x > 0.0f) return 1.0f;
    else if(x == 0.0f) return 0.0f;
    else return -1.0f;
}

internal SoundClip LoadWaveFile(char *filename)
{
    SoundClip result = {};

    FileResult fileResult = PlatformReadWholeFile(filename);
    if(fileResult.contentSize != 0)
    {
        uint8_t *at = (uint8_t *)fileResult.content;
        WaveHeader *header = (WaveHeader *)at;
        Assert(header->riffId == WAVE_RIFF);
        Assert(header->waveId == WAVE_WAVE);
        at += sizeof(WaveHeader);
        WaveChunk *chunk = (WaveChunk *)at;
        if(chunk->id == WAVE_FMT)
        {
            at += sizeof(WaveChunk);
            WaveFmt *fmt = (WaveFmt *)at;
            result.sampleCount = fmt->nSamplesPerSec;
            at += chunk->size;
        }
        chunk = (WaveChunk *)at;
        if(chunk->id == WAVE_DATA)
        {
            at += sizeof(WaveChunk);
            result.size = chunk->size;
            result.memory = (void *)at;
            at += chunk->size;
        }
    }

   return result;
}

internal void GameUpdateAndRender(GameMemory *memory, GameInput *input,
                                uint32_t width, uint32_t height)
{
    GameState *gameState = (GameState *)memory->permenantStorage;
    RenderState *renderer = (RenderState *)((uint8_t *)gameState + sizeof(GameState));
    if(!memory->initialized)
    {
        gameState->hitAudio = LoadWaveFile("hit.wav");
        PlayAudio(gameState->hitAudio);

        gameState->playerX[0] = 20.0f;
        gameState->playerY[0] = height * 0.5f;
        gameState->playerX[1] = width - gameState->playerX[0];
        gameState->playerY[1] = height - gameState->playerY[0];
        gameState->ballX = width * 0.5f;
        gameState->ballY = height * 0.5f;
        gameState->ballVelX = 2.5f;
        gameState->ballVelY = 2.5f;

        SetupRenderer(renderer);

        memory->initialized = true;
    }

    if(input->up.isDown)
    {
        gameState->playerY[0] -= 5.0f;
        gameState->playerY[1] += 5.0f;
    }
    if(input->down.isDown)
    {
        gameState->playerY[0] += 5.0f;
        gameState->playerY[1] -= 5.0f;
    }
    gameState->ballX += gameState->ballVelX;
    gameState->ballY += gameState->ballVelY;
    if(gameState->ballX - 7.0f < 0)
    {
        gameState->ballX = 7.0f;
        gameState->ballVelX *= -1.0f;
    }
    if(gameState->ballY - 7.0f < 0)
    {
        gameState->ballY = 7.0f;
        gameState->ballVelY *= -1.0f;
    }
    if(gameState->ballX + 7.0f > width)
    {
        gameState->ballX = width - 7.0f;
        gameState->ballVelX *= -1.0f;
    }
    if(gameState->ballY + 7.0f > height)
    {
        gameState->ballY = height - 7.0f;
        gameState->ballVelY *= -1.0f;
    }
    for(size_t i=0; i<2; ++i)
    {
        real32 dx = (gameState->ballX) - (gameState->playerX[i]);
        real32 dy = (gameState->ballY) - (gameState->playerY[i]);
        real32 ox = 17.0f - fabsf(dx);
        real32 oy = 57.0f - fabsf(dy);
        if(ox > 0.0f && oy > 0.0f)
        {
            PlayAudio(gameState->hitAudio);
            if(ox < oy)
            {
                gameState->ballX += Sign(dx) * ox;
                gameState->ballVelX *= -1.0f;
            }
            else
            {
                gameState->ballY += Sign(dy) * oy;
                gameState->ballVelY *= -1.0f;
            }
        }
    }

    //Draw Call
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(renderer->basicShader.program);
    for(size_t i=0; i<2; ++i)
    {
        DrawRectangle(renderer, gameState->playerX[i]-10.0f, gameState->playerY[i]-50.0f, 20.0f, 100.0f);
    }
    DrawRectangle(renderer, 300.0f, 300.0f, 200.0f, 200.0f);
    DrawRectangle(renderer, gameState->ballX-7.0f, gameState->ballY-7.0f, 14.0f, 14.0f);
    Flush(renderer);
}