#include "Renderer.h"
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>
#include "math/Util.h"

Renderer::Renderer(SDL_Window* window, SDL_Surface* surface, SDL_Renderer* renderer, Fonts* fonts)
{
    _window = window;
    _display = surface;
    _renderer = renderer;
    _fonts = fonts;

    SDL_GetWindowSize(_window, &_windowWidth, &_windowHeight);

    //Confirm that SDL is using a compatible backend. Needed for ImGui init
    SDL_GetRendererInfo(_renderer, &Info);
    std::string backend = Info.name;
    if (backend == "direct3d")
        BackendType = Direct3D;
    else if (backend == "opengl")
        BackendType = OpenGL;
    else if (backend == "vulkan")
        BackendType == Vulkan;
    else if (backend == "metal")
        BackendType == Metal;
    else if (backend == "software")
        BackendType == Software;

    //Todo: Support other backends. Only supporting OpenGL to start since ImGui requires some backend specific code and all platforms have OpenGL.
    if(BackendType != OpenGL)
        throw std::runtime_error("SDL is using an unsupported backend type '" + backend + "'.");

    //Init imgui
    InitImGui();
}

void Renderer::NewFrame()
{
    //Reset per frame data
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(_window);
    ImGui::NewFrame();

    //Clear screen by setting all pixels to black
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderClear(_renderer);

    //Clear texture data after a few frames to free up memory. Not done in debug builds since it causes an assert in dear imgui to fail.
    //This reduces RAM usage by ~40MB
#ifndef DEBUG_BUILD
    if (_frameCount < 10)
        _frameCount++;
    else
        ImGui::GetIO().Fonts->ClearTexData();
#endif
}

void Renderer::Update(f32 deltaTime)
{
    //Render gui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //Render frame
    SDL_RenderPresent(_renderer);
    ImGui::EndFrame();
}

void Renderer::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::HandleWindowResize(i32 newWidth, i32 newHeight)
{
    _windowWidth = newWidth;
    _windowHeight = newHeight;
}

void Renderer::DrawLine(const Vec2<f32>& begin, const Vec2<f32> end, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_RenderDrawLineF(_renderer, begin.x, begin.y, end.x, end.y);
}

void Renderer::DrawRectangle(const Vec2<f32>& min, const Vec2<f32>& size, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_FRect rect = { min.x, min.y, size.x, size.y };
    SDL_RenderDrawRectF(_renderer, &rect);
}

void Renderer::DrawRectangleCentered(const Vec2<f32>& center, const Vec2<f32>& size, const Vec4<u8>& color)
{
    const Vec2<f32> min = center - (size / 2.0f);
    DrawRectangle(min, size, color);
}

void Renderer::DrawRectangleFilled(const Vec2<f32>& min, const Vec2<f32>& size, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_FRect rect = { min.x, min.y, size.x, size.y };
    SDL_RenderFillRectF(_renderer, &rect);
}

void Renderer::DrawRectangleFilledCentered(const Vec2<f32>& center, const Vec2<f32>& size, const Vec4<u8>& color)
{
    const Vec2<f32> min = center - (size / 2.0f);
    DrawRectangleFilled(min, size, color);
}

void Renderer::DrawTriangle(const Vec2<f32>& pos, f32 size, f32 angle, const Vec4<u8>& color)
{
    //Calculate points
    Vec2<f32> p0 = pos + Vec2<f32>{ size, 0 };
    Vec2<f32> p1 = pos + Vec2<f32>{ -size, size * 0.75f };
    Vec2<f32> p2 = pos + Vec2<f32>{ -size, -size * 0.75f };

    //Rotate points around triangle center
    const Vec2<f32> origin = pos;
    p0.Rotate(origin, angle);
    p1.Rotate(origin, angle);
    p2.Rotate(origin, angle);

    //Draw lines between points
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_RenderDrawLineF(_renderer, p0.x, p0.y, p1.x, p1.y);
    SDL_RenderDrawLineF(_renderer, p1.x, p1.y, p2.x, p2.y);
    SDL_RenderDrawLineF(_renderer, p2.x, p2.y, p0.x, p0.y);
}

void Renderer::DrawArc(const Vec2<f32>& start, f32 length, f32 angleDegrees, f32 widthDegrees, const Vec4<u8>& color, const u32 numPoints)
{
    const f32 angleRadians = ToRadians(angleDegrees);
    const f32 widthRadians = ToRadians(std::min(widthDegrees, 360.0f));
    const f32 startAngleRadians = angleRadians - widthRadians / 2.0f;
    auto CalculateArcPoint = [&](f32 angle) -> Vec2<f32>
    {
        return start + Vec2<f32>(cos(angle), sin(angle)) * length;
    };

    //Draw arc
    Vec2<f32> last = start;
    Vec2<f32> current = CalculateArcPoint(startAngleRadians);
    for (u32 i = 0; i < numPoints; i++)
    {
        DrawLine(last, current, color);
        last = current;
        current = CalculateArcPoint(startAngleRadians + i * (widthRadians / numPoints));
    }
    DrawLine(last, current, color);
    DrawLine(current, start, color);
}

void Renderer::DrawCircle(const Vec2<f32>& position, f32 radius, const Vec4<u8>& color, const u32 numPoints)
{
    const f32 angleTotalRadians = ToRadians(360.0f);
    const f32 anglePerStepRadians = angleTotalRadians / numPoints;
    auto CalculatePoint = [&](f32 angle) -> Vec2<f32>
    {
        return position + Vec2<f32>(cos(angle), sin(angle)) * radius;
    };

    const Vec2<f32> first = CalculatePoint(0.0f);
    Vec2<f32> last = first;
    Vec2<f32> current = CalculatePoint(anglePerStepRadians);
    for (u32 i = 2; i <= numPoints; i++)
    {
        DrawLine(last, current, color);
        last = current;
        current = CalculatePoint(i * anglePerStepRadians);
    }
    DrawLine(last, first, color);
}

void Renderer::DrawPoint(const Vec2<f32>& pos, f32 size, const Vec4<u8>& color)
{
    DrawRectangleFilledCentered(pos, { size, size }, color);
}

void Renderer::InitImGui()
{
    //Create and config ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((f32)_display->w, (f32)_display->h);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

    //Load custom fonts
    _fonts->LoadFonts();

    //Initialize imgui backend
    ImGui_ImplSDL2_InitForOpenGL(_window, SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL3_Init("#version 130");

    //Set custom theme
    ImGui::StyleColorsDark();
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->WindowPadding = { 8, 8 };
    style->FramePadding = { 5, 5 };
    style->ItemSpacing = { 8, 8 };
    style->ItemInnerSpacing = { 8, 6 };
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 18.0f;
    style->GrabMinSize = 12.0f;
    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;
    style->FrameBorderSize = 1.0f;
    style->TabBorderSize = 0.0f;
    style->WindowRounding = 4.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 4.0f;
    style->PopupRounding = 4.0f;
    style->ScrollbarRounding = 4.0f;
    style->GrabRounding = 4.0f;
    style->TabRounding = 4.0f;

    colors[ImGuiCol_Text] = { 0.96f, 0.96f, 0.99f, 1.00f };
    colors[ImGuiCol_TextDisabled] = { 0.50f, 0.50f, 0.50f, 1.00f };
    colors[ImGuiCol_WindowBg] = { 0.114f, 0.114f, 0.125f, 1.0f };
    colors[ImGuiCol_ChildBg] = { 0.106f, 0.106f, 0.118f, 1.0f };
    colors[ImGuiCol_PopupBg] = { 0.09f, 0.09f, 0.10f, 1.00f };
    colors[ImGuiCol_Border] = { 0.216f, 0.216f, 0.216f, 1.0f };
    colors[ImGuiCol_BorderShadow] = { 0.00f, 0.00f, 0.00f, 0.00f };
    colors[ImGuiCol_FrameBg] = { 0.161f, 0.161f, 0.176f, 1.00f };
    colors[ImGuiCol_FrameBgHovered] = { 0.216f, 0.216f, 0.235f, 1.00f };
    colors[ImGuiCol_FrameBgActive] = { 0.255f, 0.255f, 0.275f, 1.00f };
    colors[ImGuiCol_TitleBg] = { 0.157f, 0.157f, 0.157f, 1.0f };
    colors[ImGuiCol_TitleBgActive] = { 0.216f, 0.216f, 0.216f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = { 0.157f, 0.157f, 0.157f, 1.0f };
    colors[ImGuiCol_MenuBarBg] = { 0.157f, 0.157f, 0.157f, 1.0f };
    colors[ImGuiCol_ScrollbarBg] = { 0.074f, 0.074f, 0.074f, 1.0f };
    colors[ImGuiCol_ScrollbarGrab] = { 0.31f, 0.31f, 0.32f, 1.00f };
    colors[ImGuiCol_ScrollbarGrabHovered] = { 0.41f, 0.41f, 0.42f, 1.00f };
    colors[ImGuiCol_ScrollbarGrabActive] = { 0.51f, 0.51f, 0.53f, 1.00f };
    colors[ImGuiCol_CheckMark] = { 0.44f, 0.44f, 0.47f, 1.00f };
    colors[ImGuiCol_SliderGrab] = { 0.44f, 0.44f, 0.47f, 1.00f };
    colors[ImGuiCol_SliderGrabActive] = { 0.59f, 0.59f, 0.61f, 1.00f };
    colors[ImGuiCol_Button] = { 0.20f, 0.20f, 0.22f, 1.00f };
    colors[ImGuiCol_ButtonHovered] = { 0.44f, 0.44f, 0.47f, 1.00f };
    colors[ImGuiCol_ButtonActive] = { 0.59f, 0.59f, 0.61f, 1.00f };
    colors[ImGuiCol_Header] = { 0.20f, 0.20f, 0.22f, 1.00f };
    colors[ImGuiCol_HeaderHovered] = { 0.44f, 0.44f, 0.47f, 1.00f };
    colors[ImGuiCol_HeaderActive] = { 0.59f, 0.59f, 0.61f, 1.00f };
    colors[ImGuiCol_Separator] = { 1.00f, 1.00f, 1.00f, 0.20f };
    colors[ImGuiCol_SeparatorHovered] = { 0.44f, 0.44f, 0.47f, 0.39f };
    colors[ImGuiCol_SeparatorActive] = { 0.44f, 0.44f, 0.47f, 0.59f };
    colors[ImGuiCol_ResizeGrip] = { 0.26f, 0.59f, 0.98f, 0.00f };
    colors[ImGuiCol_ResizeGripHovered] = { 0.26f, 0.59f, 0.98f, 0.00f };
    colors[ImGuiCol_ResizeGripActive] = { 0.26f, 0.59f, 0.98f, 0.00f };
    colors[ImGuiCol_Tab] = { 0.21f, 0.21f, 0.24f, 1.00f };
    colors[ImGuiCol_TabHovered] = { 0.23f, 0.514f, 0.863f, 1.0f };
    colors[ImGuiCol_TabActive] = { 0.23f, 0.514f, 0.863f, 1.0f };
    colors[ImGuiCol_TabUnfocused] = { 0.21f, 0.21f, 0.24f, 1.00f };
    colors[ImGuiCol_TabUnfocusedActive] = { 0.23f, 0.514f, 0.863f, 1.0f };
    colors[ImGuiCol_DockingPreview] = { 0.23f, 0.514f, 0.863f, 0.776f };
    colors[ImGuiCol_DockingEmptyBg] = { 0.114f, 0.114f, 0.125f, 1.0f };
    colors[ImGuiCol_PlotLines] = { 0.96f, 0.96f, 0.99f, 1.00f };
    colors[ImGuiCol_PlotLinesHovered] = { 0.12f, 1.00f, 0.12f, 1.00f };
    colors[ImGuiCol_PlotHistogram] = { 0.23f, 0.51f, 0.86f, 1.00f };
    colors[ImGuiCol_PlotHistogramHovered] = { 0.12f, 1.00f, 0.12f, 1.00f };
    colors[ImGuiCol_TextSelectedBg] = { 0.26f, 0.59f, 0.98f, 0.35f };
    colors[ImGuiCol_DragDropTarget] = { 0.26f, 0.59f, 0.98f, 0.00f };
    colors[ImGuiCol_NavHighlight] = { 0.26f, 0.59f, 0.98f, 1.00f };
    colors[ImGuiCol_NavWindowingHighlight] = { 1.00f, 1.00f, 1.00f, 0.70f };
    colors[ImGuiCol_NavWindowingDimBg] = { 0.80f, 0.80f, 0.80f, 0.20f };
    colors[ImGuiCol_ModalWindowDimBg] = { 0.80f, 0.80f, 0.80f, 0.35f };
}