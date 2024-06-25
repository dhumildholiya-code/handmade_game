#include "game.h"

const char *vertex_shader =
    "#version 400\n"
    "uniform mat4 proj;\n"
    "in vec3 vp;\n"
    "void main() {\n"
    "  gl_Position = proj * vec4(vp, 1.0);\n"
    "}";
const char *fragment_shader =
    "#version 400\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  color = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}";

inline internal real32 Sign(real32 x)
{
    if(x > 0.0f) return 1.0f;
    else if(x == 0.0f) return 0.0f;
    else return -1.0f;
}

internal void CreateOrthoProj(Matrix4X4 *mat, real32 l, real32 r, real32 t, real32 b,
                                   real32 n, real32 f)
{
    real32 rl = r - l;
    real32 tb = t - b;
    real32 fn = f - n;
    real32 irl = 1.0f / rl;
    real32 itb = 1.0f / tb;
    real32 ifn = 1.0f / fn;
    mat->data[0] = 2.0f * irl;
    mat->data[5] = 2.0f * itb;
    mat->data[10] = -2.0f * ifn;
    mat->data[12] = (r + l) * -irl;
    mat->data[13] = (t + b) * -itb;
    mat->data[14] = (f + n) * -ifn;
    mat->data[15] = 1.0f;
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

internal void DrawRectangle(RenderState *renderer, real32 x, real32 y, real32 w, real32 h)
{
    uint32_t vertexCount = 4;
    uint32_t indexCount = 6;
    real32 vertices[12] = {
        x , y, 0.0f,
        x, y+h, 0.0f,
        x+w, y+h, 0.0f,
        x+w, y, 0.0f
    };
    uint32_t indices[6] = {
        0,1,3,
        3,1,2
    };
    for(size_t i=0; i<6; ++i)
    {
        indices[i] += renderer->vertexId;
    }

    uint32_t vertOffsetBytes = renderer->vertexId*3*sizeof(real32);
    uint32_t indexOffsetBytes = renderer->indexId*sizeof(uint32_t);

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, vertOffsetBytes, vertexCount*3*sizeof(real32), vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexCount*sizeof(uint32_t), indices);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->vertexId += vertexCount;
    renderer->indexId += indexCount;
}

internal void Flush(RenderState *renderer)
{
    glBindVertexArray(renderer->vao);

    int32_t location = glGetUniformLocation(renderer->basicShader.program, "proj");
    glUniformMatrix4fv(location, 1, GL_FALSE, renderer->orthoProj.data);
    glDrawElements(GL_TRIANGLES, renderer->indexId, GL_UNSIGNED_INT, 0);

    renderer->vertexId = 0;
    renderer->indexId = 0;
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
        gameState->playerY[0] = height * 0.5f - 50.0f;
        gameState->playerX[1] = width - gameState->playerX[0] - 20.0f;
        gameState->playerY[1] = height - gameState->playerY[0] - 100.0f;
        gameState->ballX = width * 0.5f - 7.0f;
        gameState->ballY = height * 0.5f - 7.0f;
        gameState->ballVelX = 2.5f;
        gameState->ballVelY = 2.5f;

        renderer->vertexId = 0;
        renderer->indexId = 0;
        CreateOrthoProj(&renderer->orthoProj, 0.0f, 800.0f, 0.0f, 600.0f,
                        0.0f, 10.0f);
        CreateAndCompileShader(&renderer->basicShader, vertex_shader, fragment_shader);
        glGenBuffers(1, &renderer->vbo);
        glGenBuffers(1, &renderer->ibo);
        glGenVertexArrays(1, &renderer->vao);
        glBindVertexArray(renderer->vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
        glBufferData(GL_ARRAY_BUFFER, 24*3* sizeof(real32), 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*3*sizeof(uint32_t), 0, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    if(gameState->ballX < 0)
    {
        gameState->ballX = 0.0f;
        gameState->ballVelX *= -1.0f;
    }
    if(gameState->ballY < 0)
    {
        gameState->ballY = 0.0f;
        gameState->ballVelY *= -1.0f;
    }
    if(gameState->ballX + 14.0f > width)
    {
        gameState->ballX = width - 14.0f;
        gameState->ballVelX *= -1.0f;
    }
    if(gameState->ballY + 14.0f > height)
    {
        gameState->ballY = height - 14.0f;
        gameState->ballVelY *= -1.0f;
    }
    for(size_t i=0; i<2; ++i)
    {
        real32 dx = (gameState->ballX + 7.0f) - (gameState->playerX[i] + 10.0f);
        real32 dy = (gameState->ballY + 7.0f) - (gameState->playerY[i] + 50.0f);
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(renderer->basicShader.program);
    for(size_t i=0; i<2; ++i)
    {
        DrawRectangle(renderer, gameState->playerX[i], gameState->playerY[i], 20.0f, 100.0f);
    }
    DrawRectangle(renderer, gameState->ballX, gameState->ballY, 14.0f, 14.0f);
    Flush(renderer);
}