#define GLEW_STATIC
#include "GL/glew.h"

const char *vertex_shader =
    "#version 400\n"
    "layout(location = 0) in vec3 vp;\n"
    "layout(location = 1) in vec2 texCoord;\n"
    "out vec2 uv;\n"
    "uniform mat4 u_Proj;\n"
    "void main() {\n"
    "   uv = texCoord;\n"
    "   gl_Position = u_Proj * vec4(vp, 1.0);\n"
    "}";

const char *fragment_shader =
    "#version 400\n"
    "out vec4 color;\n"
    "in vec2 uv;\n"
    "uniform sampler2D u_Texture;\n"
    "void main() {\n"
    "   vec4 texColor = texture(u_Texture, uv);"
    "   color = texColor;\n"
    "}";

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

internal Texture CreateTexture(char* filename)
{
    Texture texture = {};
    stbi_set_flip_vertically_on_load(true);
    FileResult fileResult = PlatformReadWholeFile(filename);
    uint8_t *data = stbi_load_from_memory((uint8_t *)fileResult.content, fileResult.contentSize,
                                         &texture.width, &texture.height, &texture.bpp, 4);

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture.width, texture.height, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    PlatformFreeFileMemory(fileResult.content);
    stbi_image_free(data);
    return texture;
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
    glBufferData(GL_ARRAY_BUFFER, 40*sizeof(Vertex), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 20*3*sizeof(uint32_t), 0, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(Vec3)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->wood = CreateTexture("wood.png");
}

internal void Flush(RenderState *renderer)
{
    glBindVertexArray(renderer->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->wood.id);

    int32_t projLocation = glGetUniformLocation(renderer->basicShader.program, "u_Proj");
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, renderer->orthoProj.data);
    int32_t texLocation = glGetUniformLocation(renderer->basicShader.program, "u_Texture");
    glUniform1i(texLocation, 0);

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
    vertices[0].uv = {0.0f, 1.0f};
    vertices[1].position = {x, y+h, 0.0f,};
    vertices[1].uv = {0.0f, 0.0f};
    vertices[2].position = {x+w, y+h, 0.0f,};
    vertices[2].uv = {1.0f, 0.0f};
    vertices[3].position = {x+w, y, 0.0f};
    vertices[3].uv = {1.0f, 1.0f};
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