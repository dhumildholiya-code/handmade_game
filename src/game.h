#include <cstdint>
#include <math.h>

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
    uint32_t Vs;
    uint32_t Fs;
    uint32_t Program;
};
internal void CreateAndCompileShader(Shader *shader, const char* vertexShader,
                                    const char* fragShader);
struct FileResult
{
    uint32_t ContentSize;
    void *Content;
};                       
/*
NOTE: This is Debug File I/O.
This Allocates memory other than GameMemory.
*/
internal FileResult PlatformReadWholeFile(char* filename);
internal void PlatformFreeFileMemory(void *memory);
internal bool PlatformWriteWholeFile(char *filename, uint32_t memorySize, void *memory);

struct GameMemory
{
    bool Initialized;
    uint64_t PermenantStorageSize;
    void *PermenantStorage;
    uint64_t TransientStorageSize;
    void *TransientStorage;
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

// NOTE: Game Stuff.
struct Matrix4X4
{
    real32 Data[16];
};

struct GameState
{
    real32 PlayerX[2];
    real32 PlayerY[2];
    real32 BallX;
    real32 BallY;
    real32 BallVelX;
    real32 BallVelY;
};

struct RenderState
{
    Matrix4X4 OrthoProj;
    Shader BasicShader;
    uint32_t VertexId;
    uint32_t IndexId;
    uint32_t Ibo;
    uint32_t Vbo;
    uint32_t Vao;
};