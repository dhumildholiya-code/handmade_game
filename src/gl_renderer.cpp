#define GLEW_STATIC
#include "GL/glew.h"

const char *vertex_shader =
    "#version 400\n"
    "uniform mat4 proj;\n"
    "layout(location = 0) in vec3 vp;\n"
    "layout(location = 1) in vec2 i_uv;\n"
    "out vec2 o_uv;\n"
    "void main() {\n"
    "   o_uv = i_uv;\n"
    "   gl_Position = proj * vec4(vp, 1.0);\n"
    "}";

const char *fragment_shader =
    "#version 400\n"
    "out vec4 color;\n"
    "in vec2 o_uv;\n"
    "void main() {\n"
    "   color = vec4(o_uv.x, o_uv.y, 0.0, 1.0);\n"
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

internal void SetupRenderer(RenderState *renderer)
{
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
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(Vertex), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*3*sizeof(uint32_t), 0, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(Vec3)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

internal void DrawRectangle(RenderState *renderer, real32 x, real32 y, real32 w, real32 h)
{
    uint32_t vertexCount = 4;
    uint32_t indexCount = 6;
    Vertex vertices[4];
    vertices[0].position = {x , y, 0.0f};
    vertices[0].uv = {0.0f, 0.0f};
    vertices[1].position = {x, y+h, 0.0f,};
    vertices[1].uv = {0.0f, 1.0f};
    vertices[2].position = {x+w, y+h, 0.0f,};
    vertices[2].uv = {1.0f, 1.0f};
    vertices[3].position = {x+w, y, 0.0f};
    vertices[3].uv = {1.0f, 0.0f};
    uint32_t indices[6] = {
        0,1,3,
        3,1,2
    };
    for(size_t i=0; i<6; ++i)
    {
        indices[i] += renderer->vertexId;
    }

    uint32_t vertOffsetBytes = renderer->vertexId*sizeof(Vertex);
    uint32_t indexOffsetBytes = renderer->indexId*sizeof(uint32_t);

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, vertOffsetBytes, vertexCount*sizeof(Vertex), &vertices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexCount*sizeof(uint32_t), indices);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->vertexId += vertexCount;
    renderer->indexId += indexCount;
}