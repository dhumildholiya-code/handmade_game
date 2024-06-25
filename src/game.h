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
    uint32_t vs;
    uint32_t fs;
    uint32_t program;
};
internal void CreateAndCompileShader(Shader *shader, const char* vertexShader,
                                    const char* fragShader);

struct SoundClip
{
    uint32_t sampleCount;
    uint32_t size;
    void *memory;
};
internal void PlayAudio(SoundClip clip);

struct FileResult
{
    uint32_t contentSize;
    void *content;
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
    bool initialized;
    uint64_t permenantStorageSize;
    void *permenantStorage;
    uint64_t transientStorageSize;
    void *transientStorage;
};

// NOTE: Service Provided By Game To Platform.
struct ButtonState
{
    bool isDown;
};
struct GameInput
{
    ButtonState up;
    ButtonState down;
};

internal void GameUpdateAndRender(GameMemory *memory, GameInput *input, uint32_t width, uint32_t height);

// NOTE: Game Stuff.
struct Matrix4X4
{
    real32 data[16];
};

#pragma pack(push, 1)
struct WaveHeader
{
    uint32_t riffId;
    uint32_t size;
    uint32_t waveId;
};
#define RIFF_CODE(a, b, c, d) (((uint32_t)(a)<<0) | ((uint32_t)(b)<<8) | ((uint32_t)(c)<<16)| ((uint32_t)(d)<<24))
enum
{
    WAVE_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_FMT = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
    WAVE_DATA = RIFF_CODE('d', 'a', 't', 'a'),
};
struct WaveChunk
{
    uint32_t id;
    uint32_t size;
};
struct WaveFmt
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
    uint16_t wValidBitsPerSample;
    uint32_t dwChannelMask;
    uint8_t subFormat[16];
};
#pragma pack(pop)

internal SoundClip LoadWaveFile(char *filename);

struct GameState
{
    real32 playerX[2];
    real32 playerY[2];
    real32 ballX;
    real32 ballY;
    real32 ballVelX;
    real32 ballVelY;

    SoundClip hitAudio;
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