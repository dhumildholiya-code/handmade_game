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
    mat->Data[0] = 2.0f * irl;
    mat->Data[5] = 2.0f * itb;
    mat->Data[10] = -2.0f * ifn;
    mat->Data[12] = (r + l) * -irl;
    mat->Data[13] = (t + b) * -itb;
    mat->Data[14] = (f + n) * -ifn;
    mat->Data[15] = 1.0f;
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
        indices[i] += renderer->VertexId;
    }

    uint32_t vertOffsetBytes = renderer->VertexId*3*sizeof(real32);
    uint32_t indexOffsetBytes = renderer->IndexId*sizeof(uint32_t);

    glBindVertexArray(renderer->Vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->Vbo);
    glBufferSubData(GL_ARRAY_BUFFER, vertOffsetBytes, vertexCount*3*sizeof(real32), vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->Ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexCount*sizeof(uint32_t), indices);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->VertexId += vertexCount;
    renderer->IndexId += indexCount;
}

internal void Flush(RenderState *renderer)
{
    glBindVertexArray(renderer->Vao);

    int32_t location = glGetUniformLocation(renderer->BasicShader.Program, "proj");
    glUniformMatrix4fv(location, 1, GL_FALSE, renderer->OrthoProj.Data);
    glDrawElements(GL_TRIANGLES, renderer->IndexId, GL_UNSIGNED_INT, 0);

    renderer->VertexId = 0;
    renderer->IndexId = 0;
}

internal void GameUpdateAndRender(GameMemory *memory, GameInput *input,
                                uint32_t width, uint32_t height)
{
    GameState *gameState = (GameState *)memory->PermenantStorage;
    RenderState *renderer = (RenderState *)((uint8_t *)gameState + sizeof(GameState));
    if(!memory->Initialized)
    {
        gameState->PlayerX[0] = 20.0f;
        gameState->PlayerY[0] = height * 0.5f - 50.0f;
        gameState->PlayerX[1] = width - gameState->PlayerX[0] - 20.0f;
        gameState->PlayerY[1] = height - gameState->PlayerY[0] - 100.0f;
        gameState->BallX = width * 0.5f - 7.0f;
        gameState->BallY = height * 0.5f - 7.0f;
        gameState->BallVelX = 0.05f;
        gameState->BallVelY = 0.05f;

        renderer->VertexId = 0;
        renderer->IndexId = 0;
        CreateOrthoProj(&renderer->OrthoProj, 0.0f, 800.0f, 0.0f, 600.0f,
                        0.0f, 10.0f);
        CreateAndCompileShader(&renderer->BasicShader, vertex_shader, fragment_shader);
        glGenBuffers(1, &renderer->Vbo);
        glGenBuffers(1, &renderer->Ibo);
        glGenVertexArrays(1, &renderer->Vao);
        glBindVertexArray(renderer->Vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->Vbo);
        glBufferData(GL_ARRAY_BUFFER, 24*3* sizeof(real32), 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->Ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*3*sizeof(uint32_t), 0, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        memory->Initialized = true;
    }

    if(input->Up.IsDown)
    {
        gameState->PlayerY[0] -= 0.05f;
        gameState->PlayerY[1] += 0.05f;
    }
    if(input->Down.IsDown)
    {
        gameState->PlayerY[0] += 0.05f;
        gameState->PlayerY[1] -= 0.05f;
    }
    gameState->BallX += gameState->BallVelX;
    gameState->BallY += gameState->BallVelY;
    if(gameState->BallX < 0)
    {
        gameState->BallX = 0.0f;
        gameState->BallVelX *= -1.0f;
    }
    if(gameState->BallY < 0)
    {
        gameState->BallY = 0.0f;
        gameState->BallVelY *= -1.0f;
    }
    if(gameState->BallX > width)
    {
        gameState->BallX = width;
        gameState->BallVelX *= -1.0f;
    }
    if(gameState->BallY > height)
    {
        gameState->BallY = height;
        gameState->BallVelY *= -1.0f;
    }
    for(size_t i=0; i<2; ++i)
    {
        real32 dx = (gameState->BallX + 7.0f) - (gameState->PlayerX[i] + 10.0f);
        real32 dy = (gameState->BallY + 7.0f) - (gameState->PlayerY[i] + 50.0f);
        real32 ox = 17.0f - fabsf(dx);
        real32 oy = 57.0f - fabsf(dy);
        if(ox > 0.0f && oy > 0.0f)
        {
            if(ox < oy)
            {
                gameState->BallX += Sign(dx) * ox;
                gameState->BallVelX *= -1.0f;
            }
            else
            {
                gameState->BallY += Sign(dy) * oy;
                gameState->BallVelY *= -1.0f;
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(renderer->BasicShader.Program);
    for(size_t i=0; i<2; ++i)
    {
        DrawRectangle(renderer, gameState->PlayerX[i], gameState->PlayerY[i], 20.0f, 100.0f);
    }
    DrawRectangle(renderer, gameState->BallX, gameState->BallY, 14.0f, 14.0f);
    Flush(renderer);
}