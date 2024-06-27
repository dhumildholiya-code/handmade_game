#define GLEW_STATIC
#include "GL/glew.h"

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

struct Shader
{
    uint32_t vs;
    uint32_t fs;
    uint32_t program;
};

struct RenderState
{
    Matrix4X4 orthoProj;
    Shader basicShader;
    uint32_t vertexId;
    uint32_t indexId;
    uint32_t ibo;
    uint32_t vbo;
    uint32_t vao;
};

internal void CreateAndCompileShader(Shader *shader, const char* vertexShader,
                                    const char* fragShader)
{
    shader->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader->vs, 1, &vertexShader, 0);
    glCompileShader(shader->vs);
    shader->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader->fs, 1, &fragShader, 0);
    glCompileShader(shader->fs);
    shader->program = glCreateProgram();
    glAttachShader(shader->program, shader->vs);
    glAttachShader(shader->program, shader->fs);
    glLinkProgram(shader->program);
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