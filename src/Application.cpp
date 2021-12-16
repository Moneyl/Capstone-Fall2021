#include "Application.h"
#include "utility/Platform.h"
#include "math/Util.h"
#include "Config.h"
#include "vm/VM.h"
#include "utility/Sound.h"
#include <iostream>

bool Application::Run()
{
    return Init() && MainLoop() && Shutdown();
}

bool Application::Init()
{
    //Load config file
    Config::Get()->Load();

    //Initialize SDL resources. Application owns all SDL resources and logic is split into other classes like Input and Renderer
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "SDL failed to initialize. Error: " << SDL_GetError() << "\n";
        return false;
    }

    //Create window + render surface
    _window = SDL_CreateWindow("SDL_ImGui_Testing", 50, 50, _windowWidth, _windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
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
    if (!Sound::Init())
        return false;
    Arena.Reset();
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

        UpdateKeybinds();
        Arena.Update(_deltaTime);
        Arena.Draw(&Renderer);

        //Update app logic
        Gui.Update(_deltaTime);

        //Render frame
        Renderer.Update(_deltaTime);

        //Wait for target framerate
        while (_frameTimer.ElapsedSeconds() < targetDeltaTime)
        {
            f32 elapsed = _frameTimer.ElapsedSeconds();
            f32 timeToTargetFramerateMs = (targetDeltaTime - elapsed) * 1000.0f;
            if (timeToTargetFramerateMs < targetDeltaTimeMs)
                ThreadSleep((u32)timeToTargetFramerateMs, true);
        }
        _deltaTime = _frameTimer.ElapsedSeconds();
        _frameTimer.Restart();
    }

    return true;
}

bool Application::Shutdown()
{
    //Cleanup resources
    Arena.Robots.clear();
    Sound::Shutdown();
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

void Application::UpdateKeybinds()
{
    if (Input.KeyPressed(SDL_KeyCode::SDLK_F1))
        Arena.Reset();
    if (Input.KeyPressed(SDL_KeyCode::SDLK_F2))
        Gui.TournamentCreatorVisible = !Gui.TournamentCreatorVisible;
    if (Input.KeyPressed(SDL_KeyCode::SDLK_F3))
    {
        //Toggle high speed
        if (Arena.GameSpeed <= 1.0f)
            Arena.GameSpeed = Arena::MaxGameSpeed;
        else
            Arena.GameSpeed = 1.0f;
    }
}