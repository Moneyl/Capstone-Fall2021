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

    //Create robots
    Vec2<i32> arenaCenter = ArenaSize / 2;
    Robot& robot = Robots.emplace_back();
    robot.Vm->LoadProgramFromSource(BuildConfig::AssetFolderPath + "tests/Test1.sunyat");
    robot.Position.x = arenaCenter.x + 400;
    robot.Position.y = arenaCenter.y + 50;
    Robot& robot2 = Robots.emplace_back();
    robot2.Vm->LoadProgramFromSource(BuildConfig::AssetFolderPath + "tests/Test0.sunyat");
    robot2.Position.x = arenaCenter.x + 300;
    robot2.Position.y = arenaCenter.y + 0;

    Gui = { this };

    return true;
}

bool Application::MainLoop()
{
    //Run main loop until exit is requested
    _frameTimer.Restart();
    SDL_Event event;
    while (!_quit)
    {
        //Reset per-frame data
        Input.NewFrame();
        Renderer.NewFrame();

        //Handle queued SDL events
        while (SDL_PollEvent(&event))
            HandleEvent(&event);

        //Calculate how many cycles to execute this frame
        //Only whole cycles are executed. If there's only enough time for part of a cycle it'll be accumulated for next frame
        _cycleAccumulator += _deltaTime; //Accumulate cycle time
        const f32 timeBetweenCycles = 1.0f / (f32)CyclesPerSecond;
        const u32 cyclesToExecute = truncf(_cycleAccumulator / timeBetweenCycles);
        _cycleAccumulator -= cyclesToExecute * timeBetweenCycles; //Remove time for executed cycles

        //Update robots
        for (Robot& robot : Robots)
            robot.Update(_deltaTime, cyclesToExecute);

        //Update app logic
        Gui.Update(_deltaTime);

        //Draw arena
        Renderer.DrawRectangleFilled({ 400, 50 }, ArenaSize, { 64, 64, 64, 255 }); //Floor
        Renderer.DrawRectangle({ 400, 50 }, ArenaSize, { 200, 0, 0, 255 }); //Border
        
        //Draw robots
        for (Robot& robot : Robots)
            Renderer.DrawRectangle({ robot.Position.x, robot.Position.y }, { 10, 15 }, { 0, 127, 0, 255 });

        //Render frame
        Renderer.Update(_deltaTime);

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
    Robots.clear();
    Renderer.Cleanup();
    SDL_DestroyRenderer(_rendererSDL);
    SDL_DestroyWindow(_window);
    _window = nullptr;

    SDL_Quit();
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