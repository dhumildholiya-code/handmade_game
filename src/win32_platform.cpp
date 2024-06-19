#define GLEW_STATIC
#include "GL/glew.h"
#include "game.cpp"
/*
- QueryPerformance Counter and RDTSC
- Platform independent game memory
- File I/O
- Enfore video frame rate
- Loading game code dynamically
*/

#include <windows.h>

global_var bool Running;
global_var HGLRC GlContext;

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
    default:
        result = DefWindowProc(window, message, wParam, lParam);
        break;
    }
    return result;
}

internal bool CreateGlContext(HDC deviceContext)
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                   PSTR cmdLine, int cmdShow)
{
    WNDCLASSEXA windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "PongGameClass";
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

            GameMemory gameMemory = {0};
            gameMemory.permenantStorageSize = Megabytes(2);
            gameMemory.transientStorageSize = Megabytes(64);
            uint64_t totalSize = gameMemory.permenantStorageSize + gameMemory.transientStorageSize;
            gameMemory.permenantStorage = VirtualAlloc(0, totalSize, MEM_RESERVE|MEM_COMMIT,
                                                    PAGE_READWRITE);
            gameMemory.transientStorage = (uint8_t *)gameMemory.permenantStorage
                                        + gameMemory.permenantStorageSize;

            if (CreateGlContext(deviceContext))
            {
                Running = true;
            }
            else
            {
                // TODO : Logging
            }
            while (Running)
            {
                //MessageLoop
                MSG msg;
                while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
                GameUpdateAndRender(&gameMemory);
                SwapBuffers(deviceContext);
            }
        }
        else
        {
            // TODO : Logging
        }
    }
    else
    {
        // TODO : Logging
    }
    return 0;
}