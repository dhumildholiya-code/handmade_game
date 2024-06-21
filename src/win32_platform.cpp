#define GLEW_STATIC
#include "GL/glew.h"
#include "game.cpp"

/* TODO:
- File I/O
- Pass DeltaTime to Game From Platform
- Asset loading (wav, png, ttf)
- Setup XAudio2
*/
#include <windows.h>
#include <stdio.h>

global_var bool Running;
global_var HGLRC GlContext;
global_var int64_t PerfFreq;

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
internal void CreateAndCompileShader(Shader *shader,
                                     const char *vertexShader, const char *fragShader)
{
    shader->Vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader->Vs, 1, &vertexShader, 0);
    glCompileShader(shader->Vs);
    shader->Fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader->Fs, 1, &fragShader, 0);
    glCompileShader(shader->Fs);
    shader->Program = glCreateProgram();
    glAttachShader(shader->Program, shader->Vs);
    glAttachShader(shader->Program, shader->Fs);
    glLinkProgram(shader->Program);
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

            GameInput input = {0};
            GameMemory gameMemory = {0};
            gameMemory.PermenantStorageSize = Megabytes(2);
            gameMemory.TransientStorageSize = Megabytes(64);
            uint64_t totalSize = gameMemory.PermenantStorageSize + gameMemory.TransientStorageSize;
            gameMemory.PermenantStorage = VirtualAlloc(0, totalSize, MEM_RESERVE|MEM_COMMIT,
                                                    PAGE_READWRITE);
            gameMemory.TransientStorage = (uint8_t *)gameMemory.PermenantStorage
                                        + gameMemory.PermenantStorageSize;

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
                                    input.Up.IsDown = isDown;
                                }
                                else if(vkCode == 'S')
                                {
                                    input.Down.IsDown = isDown;
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

                // int64_t counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
                real32 msPerFrame = elapsedTime * 1000.0f;
                real32 fps = 1.0f / elapsedTime;;

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