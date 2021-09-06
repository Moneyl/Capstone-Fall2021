#include "Application.h"
#include <iostream>

bool Application::Run()
{
    return Init() && MainLoop() && Shutdown();
}

bool Application::Init()
{
    //Initialize resources
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
    
    return true;
}

bool Application::MainLoop()
{
    //Run main loop until exit is requested
    SDL_Event event;
    while (!_quit)
    {
        //Handle qeued SDL events
        while (SDL_PollEvent(&event))
            HandleEvent(&event);

        //Update app logic

        //Render frame
        SDL_UpdateWindowSurface(_window);

        //Reset per-frame data
        Input.EndFrame();
    }

    return true;
}

bool Application::Shutdown()
{
    //Cleanup resources
    SDL_FreeSurface(_display);
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
    else if (event->type == SDL_EventType::SDL_WINDOWEVENT && event->window.type == SDL_WINDOWEVENT_SIZE_CHANGED)
    {
        HandleWindowResize(event->window.data1, event->window.data2);
    }
    else
    {
        Input.HandleEvent(event);
    }
}

void Application::HandleWindowResize(int newWidth, int newHeight)
{
    _windowWidth = newWidth;
    _windowHeight = newHeight;
    printf("Window resized to: {%d, %d}", _windowWidth, _windowHeight);
}