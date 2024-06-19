#include "game.h"

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

internal void GameUpdateAndRender(GameMemory *memory)
{
    GameState *gameState = (GameState *)memory->permenantStorage;
    if(!memory->initialized)
    {
        gameState->vertexId = 0;
        gameState->indexId = 0;
        CreateOrthoProj(&gameState->orthoProj, 0.0f, 800.0f, 0.0f, 600.0f,
                        0.0f, 10.0f);
        CreateAndCompileShader(&gameState->basicShader, vertex_shader, fragment_shader);
        glGenBuffers(1, &gameState->vbo);
        glGenBuffers(1, &gameState->ibo);
        glGenVertexArrays(1, &gameState->vao);
        glBindVertexArray(gameState->vao);
        glBindBuffer(GL_ARRAY_BUFFER, gameState->vbo);
        glBufferData(GL_ARRAY_BUFFER, 24*3* sizeof(real32), 0, GL_DYNAMIC_DRAW);
        // glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(real32), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gameState->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*3*sizeof(uint32_t), 0, GL_DYNAMIC_DRAW);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(uint32_t), indices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        memory->initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gameState->basicShader.program);
    DrawRectangle(gameState, 100.0f, 100.0f, 100.0f, 100.0f);
    DrawRectangle(gameState, 200.0f, 100.0f, 100.0f, 100.0f);
    DrawRectangle(gameState, 300.0f, 100.0f, 100.0f, 100.0f);
    DrawRectangle(gameState, 400.0f, 100.0f, 100.0f, 100.0f);
    DrawRectangle(gameState, 500.0f, 100.0f, 100.0f, 100.0f);
    Flush(gameState);
}

internal void DrawRectangle(GameState *gameState, real32 x, real32 y, real32 w, real32 h)
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
        indices[i] += gameState->vertexId;
    }

    uint32_t vertOffsetBytes = gameState->vertexId*3*sizeof(real32);
    uint32_t indexOffsetBytes = gameState->indexId*sizeof(uint32_t);


    glBindVertexArray(gameState->vao);
    glBindBuffer(GL_ARRAY_BUFFER, gameState->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, vertOffsetBytes, vertexCount*3*sizeof(real32), vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gameState->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexCount*sizeof(uint32_t), indices);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    gameState->vertexId += vertexCount;
    gameState->indexId += indexCount;
}

internal void Flush(GameState *gameState)
{
    glBindVertexArray(gameState->vao);

    int32_t location = glGetUniformLocation(gameState->basicShader.program, "proj");
    glUniformMatrix4fv(location, 1, GL_FALSE, gameState->orthoProj.data);
    glDrawElements(GL_TRIANGLES, gameState->indexId, GL_UNSIGNED_INT, 0);

    gameState->vertexId = 0;
    gameState->indexId = 0;
}