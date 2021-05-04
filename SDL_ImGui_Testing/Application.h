#pragma once
//Defined to prevent SDL from overriding main with a macro.
//It's apparently done for init purposes but isn't necessary if we define this macro and call SDL_SetMainReady() before calling SDL_Init()
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "utility/Input.h"

//Root class of app. Owns data that is alive for the entire app runtime.
class Application
{
public:
    bool Run();
    
    Input Input;

private:
    bool Init();
    bool MainLoop();
    bool Shutdown();
    void HandleEvent(SDL_Event* event);
    void HandleWindowResize(int newWidth, int newHeight);

    //Main loop will exit and close the app when this is true
    bool _quit = false;
    
    //Window and display surface being rendered to it
    SDL_Window* _window = nullptr;
    SDL_Surface* _display = nullptr;
    int _windowWidth = 800;
    int _windowHeight = 600;
};