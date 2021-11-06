#pragma once
#include "Typedefs.h"
//Defined to prevent SDL from overriding main with a macro.
//It's apparently done for init purposes but isn't necessary if we define this macro and call SDL_SetMainReady() before calling SDL_Init()
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "utility/Input.h"
#include "render/Renderer.h"
#include "utility/Timer.h"
#include "gui/Fonts.h"
#include "gui/Gui.h"
#include "BuildConfig.h"
#include "robots/Robot.h"
#include <vector>

//Root class of app. Owns data that is alive for the entire app runtime.
class Application
{
public:
    bool Run();
    
    Input Input;
    Renderer Renderer;
    Fonts Fonts;
    Gui Gui;
    std::vector<Robot> Robots = {};
    u32 CyclesPerFrame = 20; //# of VM cycles to run each app frame
    Vec2<i32> ArenaSize = { 1000, 1000 };

private:
    bool Init();
    bool MainLoop();
    bool Shutdown();
    void HandleEvent(SDL_Event* event);
    void HandleWindowResize(i32 newWidth, i32 newHeight);

    //Main loop will exit and close the app when this is true
    bool _quit = false;
    
    //Window and display surface being rendered to it
    SDL_Window* _window = nullptr;
    SDL_Surface* _display = nullptr;
    SDL_Renderer* _rendererSDL = nullptr;
    i32 _windowWidth = 800;
    i32 _windowHeight = 600;

    const u32 _targetFramerate = 60;
    const f32 targetDeltaTime = 1.0f / (f32)_targetFramerate;
    f32 _deltaTime = 0.0f;
    Timer _frameTimer;
};