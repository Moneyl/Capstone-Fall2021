#pragma once
#include "Typedefs.h"
#include "gui/Fonts.h"
#include "math/Vec2.h"
#include "math/Vec4.h"
#include <SDL.h>

const Vec4<u8> ColorWhite = { 255, 255, 255, 255 };

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

    //Primitive drawing functions
    void DrawLine(const Vec2<f32>& begin, const Vec2<f32> end, const Vec4<u8>& color = ColorWhite);
    void DrawRectangle(const Vec2<f32>& min, const Vec2<f32>& size, const Vec4<u8>& color = ColorWhite);
    void DrawRectangleFilled(const Vec2<f32>& min, const Vec2<f32>& size, const Vec4<u8>& color = ColorWhite);
    void DrawTriangle(const Vec2<f32>& pos, f32 size, f32 angle, const Vec4<u8>& color = ColorWhite);

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