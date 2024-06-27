#include "game.cpp"

/* TODO:
- File I/O
- Pass DeltaTime to Game From Platform
- Asset loading (wav, png, ttf)
- Setup XAudio2
*/
#include <windows.h>
#include <stdio.h>
#include <xaudio2.h>

global_var bool Running;
global_var bool IsAudioInit;
global_var int64_t PerfFreq;
global_var HGLRC GlContext;
global_var IXAudio2 *XAudio2;
global_var IXAudio2SourceVoice *AudioSource;

internal bool Win32InitOpengl(HDC deviceContext)
{
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pixelFormat, &pfd);
    GlContext = wglCreateContext(deviceContext);
    if (wglMakeCurrent(deviceContext, GlContext))
    {
        GLenum result = glewInit();
        if (result == GLEW_OK)
        {
            OutputDebugStringA((char *)glGetString(GL_VERSION));
            glClearColor(1.f, 0.0f, 1.0f, 1.0f);
            return true;
        }
        else
        {
            // TODO : Logging
            return false;
        }
    }
    else
    {
        // TODO : Logging
        return false;
    }
}

#define XAUDIO2_CREATE(name) HRESULT name(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor)
typedef XAUDIO2_CREATE(xaudio2_create);

internal void Win32InitXAudio2()
{
    HMODULE xAudio2Lib = LoadLibraryA("xaudio2_9.dll");
    if(xAudio2Lib)
    {
        xaudio2_create *XAudio2Create = (xaudio2_create *)GetProcAddress(xAudio2Lib, "XAudio2Create");
        if(XAudio2Create && SUCCEEDED(XAudio2Create(&XAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        {
            IXAudio2MasteringVoice* pMasterVoice = nullptr;
            if (SUCCEEDED(XAudio2->CreateMasteringVoice(&pMasterVoice)))
            {
                WAVEFORMATEX waveFmt = {};
                waveFmt.wFormatTag = WAVE_FORMAT_PCM;
                waveFmt.nChannels = 1;
                waveFmt.nSamplesPerSec = 44100;
                waveFmt.wBitsPerSample = 16;
                waveFmt.nBlockAlign = (waveFmt.nChannels * waveFmt.wBitsPerSample) / 8;
                waveFmt.nAvgBytesPerSec = waveFmt.nSamplesPerSec * waveFmt.nBlockAlign;

                if(SUCCEEDED(XAudio2->CreateSourceVoice(&AudioSource, &waveFmt)))
                {
                    IsAudioInit = true;
                }
                else
                {
                    //TODO: Failed to Create Source Voice.
                }
            }
            else
            {
                //TODO: Failed to Create Master Voice.
            }
        }
        else
        {
            //TODO: Failed to Create XAudio2.
        }
    }
}

internal void PlayAudio(SoundClip clip)
{
    if(IsAudioInit)
    {
        XAUDIO2_BUFFER buffer = {};
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = clip.size;
        buffer.pAudioData = (uint8_t *)clip.memory;
        if(SUCCEEDED(AudioSource->SubmitSourceBuffer(&buffer)))
        {
            AudioSource->Start(0);
        }
    }
}

internal FileResult PlatformReadWholeFile(char* filename)
{
    FileResult result = {};

    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                            OPEN_EXISTING, 0,0);
    if(file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if(GetFileSizeEx(file, &fileSize))
        {
            Assert(fileSize.QuadPart <= 0xFFFFFFFF);
            result.contentSize = (uint32_t)fileSize.QuadPart;
            result.content = VirtualAlloc(0, result.contentSize, MEM_RESERVE|MEM_COMMIT,
                                                        PAGE_READWRITE);
            if(result.content)
            {
                DWORD bytesToRead;
                if(ReadFile(file, result.content, result.contentSize, &bytesToRead, 0)
                    && bytesToRead == result.contentSize)
                {
                }
                else
                {
                    PlatformFreeFileMemory(result.content);
                }
            }
            else
            {
                //LOG: FileMemory allocation failed.
            }
        }
        else
        {
            //LOG: GetFileSize failed.
        }
        CloseHandle(file);
    }
    else
    {
        //LOG: Creating file failed.
    }

    return result;
}

internal void PlatformFreeFileMemory(void *memory)
{
    if(memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

internal bool PlatformWriteWholeFile(char *filename, uint32_t memorySize, void *memory)
{
    bool result = false;
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
    if(file != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if(WriteFile(file, memory, memorySize, &bytesWritten, 0)
            && memorySize == bytesWritten)
        {
            result = true;
        }
        else
        {
            //LOG: Not able to write file entirely.
        }
        CloseHandle(file);
    }
    else
    {
        //LOG: Creating file failed.
    }
    return result;
}

inline internal LARGE_INTEGER Win32GetWallClock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline internal real32 Win32ElapsedSecond(LARGE_INTEGER start, LARGE_INTEGER end)
{
    int64_t counterElapsed = end.QuadPart - start.QuadPart;
    return (real32)counterElapsed / (real32)PerfFreq;
}

internal LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
    case WM_CLOSE:
        Running = false;
        break;
    case WM_DESTROY:
        Running = false;
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        Assert(!"Keyboard input came in through carzy way.");
    } break;
    default:
        result = DefWindowProc(window, message, wParam, lParam);
        break;
    }
    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                   PSTR cmdLine, int cmdShow)
{
    WNDCLASSEXA windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "PongGameClass";

    LARGE_INTEGER perfFreqResult;
    QueryPerformanceFrequency(&perfFreqResult);
    PerfFreq = perfFreqResult.QuadPart;
    int targetFps = 60;
    real32 targetElapsedTime = 1.0f / (real32)targetFps;

    if (RegisterClassExA(&windowClass))
    {
        HWND window = CreateWindowExA(0, windowClass.lpszClassName,
                                      "Pong Game",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      800, 600,
                                      0, 0, instance, 0);
        if (window)
        {
            HDC deviceContext = GetDC(window);
            Win32InitXAudio2();

            GameInput input = {0};
            GameMemory gameMemory = {0};
            gameMemory.permenantStorageSize = Megabytes(2);
            gameMemory.transientStorageSize = Megabytes(64);
            uint64_t totalSize = gameMemory.permenantStorageSize + gameMemory.transientStorageSize;
            gameMemory.permenantStorage = VirtualAlloc(0, totalSize, MEM_RESERVE|MEM_COMMIT,
                                                    PAGE_READWRITE);
            gameMemory.transientStorage = (uint8_t *)gameMemory.permenantStorage
                                        + gameMemory.permenantStorageSize;

            if (Win32InitOpengl(deviceContext))
            {
                Running = true;
            }
            else
            {
                // TODO : OpenGl Init failed
            }
            int64_t lastCycleCount = __rdtsc();
            LARGE_INTEGER lastCounter = Win32GetWallClock();
            while (Running)
            {
                MSG msg;
                while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                {
                    switch(msg.message)
                    {
                        case WM_QUIT:
                        {
                            Running = false;
                        } break;
                        case WM_KEYDOWN:
                        case WM_KEYUP:
                        case WM_SYSKEYDOWN:
                        case WM_SYSKEYUP:
                        {
                            uint32_t vkCode = (uint32_t)msg.wParam;
                            bool wasDown = (msg.lParam & (1 << 30)) != 0;
                            bool isDown = (msg.lParam & (1 << 31)) == 0;
                            if(wasDown != isDown)
                            {
                                if(vkCode == 'W')
                                {
                                    input.up.isDown = isDown;
                                }
                                else if(vkCode == 'S')
                                {
                                    input.down.isDown = isDown;
                                }
                            }
                        } break;
                        default:
                        {
                            TranslateMessage(&msg);
                            DispatchMessageA(&msg);
                        } break;
                    }
                }
                GameUpdateAndRender(&gameMemory, &input, 800, 600);

                LARGE_INTEGER endCounter = Win32GetWallClock();
                lastCounter = endCounter;
                real32 elapsedTime = Win32ElapsedSecond(lastCounter, endCounter);
                while(elapsedTime < targetElapsedTime)
                {
                    endCounter = Win32GetWallClock();
                    elapsedTime = Win32ElapsedSecond(lastCounter, endCounter);
                }
                SwapBuffers(deviceContext);

                real32 msPerFrame = elapsedTime * 1000.0f;
                real32 fps = 1.0f / elapsedTime;

                char buffer[256];
                sprintf(buffer, "%f ms/f, %f f/s\n", msPerFrame, fps);
                OutputDebugStringA(buffer);

                int64_t endCycleCount = __rdtsc();
                int64_t cyclesElapsed = endCycleCount - lastCycleCount;
                real32 mCPerFrame = (real32)cyclesElapsed / (1000.0f * 1000.0f);
                lastCycleCount = endCycleCount;
            }
        }
        else
        {
            // TODO : Winodw creation failed.
        }
    }
    else
    {
        // TODO : Register window failedd.
    }
    return 0;
}
