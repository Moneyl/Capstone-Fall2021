#pragma once
#include "Typedefs.h"
#include "gui/Fonts.h"
#include <SDL.h>

enum RenderBackend
{
    Unknown,
    Software,
    Direct3D,
    OpenGL,
    Metal,
    Vulkan,
};

class Renderer
{
public:
    Renderer() { }
    Renderer(SDL_Window* window, SDL_Surface* surface, SDL_Renderer* renderer, Fonts* fonts);
    void NewFrame();
    void Update(f32 deltaTime);
    void Cleanup();
    void HandleWindowResize(i32 newWidth, i32 newHeight);

    SDL_RendererInfo Info;
    RenderBackend BackendType = Unknown;

private:
    void InitImGui();

    SDL_Window* _window = nullptr;
    SDL_Surface* _display = nullptr;
    SDL_Renderer* _renderer = nullptr;
    Fonts* _fonts = nullptr;
    i32 _windowWidth;
    i32 _windowHeight;
    u32 _frameCount = 0;
};