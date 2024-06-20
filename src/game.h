#include <cstdint>

#define internal static
#define local_persist static
#define global_var static

#if GAME_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Killobytes(Value) (Value * 1024)
#define Megabytes(Value) (Killobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define ArrayCount(Array) (sizeof(Array)/ sizeof(Array[0]))

typedef float real32;
typedef double real64;

// NOTE: Service Provided By Platfrom to Game.
struct Shader
{
    uint32_t vs;
    uint32_t fs;
    uint32_t program;
};
internal void CreateAndCompileShader(Shader *shader, const char* vertexShader,
                                    const char* fragShader);

struct GameMemory
{
    bool initialized;
    uint64_t permenantStorageSize;
    void *permenantStorage;
    uint64_t transientStorageSize;
    void *transientStorage;
};

// NOTE: Service Provided By Game To Platform.
struct ButtonState
{
    bool IsDown;
};
struct GameInput
{
    ButtonState Up;
    ButtonState Down;
};
internal void GameUpdateAndRender(GameMemory *memory, GameInput *input, uint32_t width, uint32_t height);

// NOTE : These is not Service To Platfrom from game.
struct Matrix4X4
{
    real32 data[16];
};

struct GameState
{
    real32 playerX;
    real32 playerY;
    Matrix4X4 orthoProj;
    Shader basicShader;
    uint32_t vertexId;
    uint32_t indexId;
    uint32_t ibo;
    uint32_t vbo;
    uint32_t vao;
};