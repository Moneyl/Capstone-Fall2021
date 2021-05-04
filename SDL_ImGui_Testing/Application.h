#pragma once
//Defined to prevent SDL from overriding main with a macro.
//It's apparently done for init purposes but isn't necessary if we define this macro and call SDL_SetMainReady() before calling SDL_Init()
#define SDL_MAIN_HANDLED
#include <SDL.h>

//Root class of app. Owns data that is alive for the entire app runtime.
class Application
{
public:
    bool Run();
    
private:
    bool Init();
    bool MainLoop();
    bool Shutdown();

    void HandleEvent(SDL_Event* event);

    //Main loop will exit when this is set to true
    bool _exitRequested = false;
    
    //Window and display surface being rendered to it
    SDL_Window* _window = nullptr;
    SDL_Surface* _display = nullptr;

    //Window dimensions
    int _windowWidth = 800;
    int _windowHeight = 600;
};