#include "Application.h"
#include "utility/Platform.h"
#include "vm/VM.h"
#include <iostream>

bool Application::Run()
{
    return Init() && MainLoop() && Shutdown();
}

bool Application::Init()
{
    //Initialize SDL resources. Application owns all SDL resources and logic is split into other classes like Input and Renderer
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "SDL failed to initialize. Error: " << SDL_GetError() << "\n";
        return false;
    }

    //Create window + render surface
    _window = SDL_CreateWindow("SDL_ImGui_Testing", 50, 50, _windowWidth, _windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!_window)
    {
        std::cerr << "SDL failed to create window. Error: " << SDL_GetError() << "\n";
        return false;
    }
    _display = SDL_GetWindowSurface(_window);

    //Todo: Support other backends
    //Only supporting OpenGL 3.3 as a backend to start since ImGui requires some backend specific code. Will support others later on.
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    //Initialize renderer
    _rendererSDL = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_rendererSDL)
    {
        std::cerr << "SDL failed to create a renderer! Error: " << SDL_GetError() << "\n";
        return false;
    }
    Renderer = { _window, _display, _rendererSDL, &Fonts };

    Gui = { this };
    Vm = new VM();
    std::string testProgramFilename = "tests/Test1.sunyat";
    Vm->LoadProgramFromSource(BuildConfig::AssetFolderPath + testProgramFilename);

    return true;
}

bool Application::MainLoop()
{
    //Run main loop until exit is requested
    _frameTimer.Restart();
    SDL_Event event;
    while (!_quit)
    {
        Renderer.NewFrame();

        //Handle queued SDL events
        while (SDL_PollEvent(&event))
            HandleEvent(&event);

        //Update VM
        for (u32 i = 0; i < VmCyclesPerFrame; i++)
        {
            Result<void, VMError> cycleResult = Vm->Cycle();
            if (cycleResult.Error())
            {
                VMError& error = cycleResult.ErrorData();
                printf("Error in VM::Cycle()! Code: %s, Message: %s\n", to_string(error.Code).c_str(), error.Message.c_str());
                //Todo: Report error and pause this VM only instead of exiting
                return false;
            }
        }

        //Update app logic
        Gui.Update(_deltaTime);

        //Render frame
        Renderer.Update(_deltaTime);

        //Reset per-frame data
        Input.EndFrame();

        //Wait for target framerate
        while (_frameTimer.ElapsedSeconds() < targetDeltaTime)
        {
            f32 timeToTargetFramerateMs = (targetDeltaTime - _frameTimer.ElapsedSeconds()) * 1000.0f;
            ThreadSleep(timeToTargetFramerateMs, true);
        }
        _deltaTime = _frameTimer.ElapsedSeconds();
        _frameTimer.Restart();
    }

    return true;
}

bool Application::Shutdown()
{
    //Cleanup resources
    Renderer.Cleanup();
    SDL_DestroyRenderer(_rendererSDL);
    SDL_DestroyWindow(_window);
    _window = nullptr;

    SDL_Quit();
    delete Vm;
    return true;
}

void Application::HandleEvent(SDL_Event* event)
{
    if (event->type == SDL_EventType::SDL_QUIT)
    {
        _quit = true;
    }
    else if (event->type == SDL_EventType::SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        HandleWindowResize(event->window.data1, event->window.data2);
    }
    else
    {
        Input.HandleEvent(event);
    }
}

void Application::HandleWindowResize(i32 newWidth, i32 newHeight)
{
    _windowWidth = newWidth;
    _windowHeight = newHeight;
    Renderer.HandleWindowResize(newWidth, newHeight);
}