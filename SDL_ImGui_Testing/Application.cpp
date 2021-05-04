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
        std::cerr << "SDL failed to initialized. Error: " << SDL_GetError() << "\n";
        return false;
    }

    //Create window + render surface
    _window = SDL_CreateWindow("SDL_ImGui_Testing", 50, 50, _windowWidth, _windowHeight, SDL_WINDOW_SHOWN);
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
    while (!_exitRequested)
    {
        //Handle qeued SDL events
        while (SDL_PollEvent(&event))
            HandleEvent(&event);

        //Update app logic


        //Render frame
        SDL_UpdateWindowSurface(_window);
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
    if (event->type == SDL_QUIT)
        _exitRequested = true;
}
