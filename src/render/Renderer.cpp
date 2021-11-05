#include "Renderer.h"
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

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

void Renderer::DrawLine(const Vec2<i32>& begin, const Vec2<i32> end, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_RenderDrawLine(_renderer, begin.x, begin.y, end.x, end.y);
}

void Renderer::DrawRectangle(const Vec2<i32>& min, const Vec2<i32>& size, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_Rect rect = { min.x, min.y, size.x, size.y };
    SDL_RenderDrawRect(_renderer, &rect);
}

void Renderer::DrawRectangleFilled(const Vec2<i32>& min, const Vec2<i32>& size, const Vec4<u8>& color)
{
    SDL_SetRenderDrawColor(_renderer, color.x, color.y, color.z, color.w);
    SDL_Rect rect = { min.x, min.y, size.x, size.y };
    SDL_RenderFillRect(_renderer, &rect);
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

    style->WindowPadding = ImVec2(8, 8);
    style->WindowRounding = 0.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 0.0f;
    style->ItemSpacing = ImVec2(8, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 18.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabMinSize = 12.0f;
    style->GrabRounding = 0.0f;
    style->TabRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->PopupRounding = 0.0f;

    style->WindowBorderSize = 1.0f;
    style->FrameBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;

    colors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.114f, 0.114f, 0.125f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.106f, 0.106f, 0.118f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.216f, 0.216f, 0.216f, 1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.161f, 0.161f, 0.176f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.216f, 0.216f, 0.235f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.255f, 0.255f, 0.275f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.216f, 0.216f, 0.216f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.074f, 0.074f, 0.074f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.53f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.47f, 0.39f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.44f, 0.44f, 0.47f, 0.59f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.23f, 0.514f, 0.863f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.514f, 0.863f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.21f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.23f, 0.514f, 0.863f, 1.0f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.23f, 0.514f, 0.863f, 0.776f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.114f, 0.114f, 0.125f, 1.0f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.12f, 1.00f, 0.12f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.23f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.12f, 1.00f, 0.12f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.91f, 0.62f, 0.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}