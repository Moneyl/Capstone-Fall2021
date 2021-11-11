#pragma once
#include "src/Typedefs.h"
#include "BuildConfig.h"
#include <imgui.h>
#include <IconsFontAwesome5_c.h>

class ImGuiFont
{
public:
    ImGuiFont(f32 size) : _size(size) {}

    ImFont* GetPtr() { return _ptr; }
    f32 Size() { return _size; }
    void Push() const { ImGui::PushFont(_ptr); }
    void Pop() const { ImGui::PopFont(); }
    void Load(const ImGuiIO& io, const ImFontConfig* fontConfig, const ImWchar* glyphRanges)
    {
        //Todo: Scale font size by window size so it looks good at all resolutions
        //Load main font
        io.Fonts->AddFontFromFileTTF(BuildConfig::MainFontPath.c_str(), _size);

        //Load FontAwesome image font and merge it with the main font
        _ptr = io.Fonts->AddFontFromFileTTF(BuildConfig::FontAwesomePath.c_str(), _size, fontConfig, glyphRanges);
    }

private:
    ImFont* _ptr = nullptr;
    f32 _size;
};

//Manages fonts for dear imgui
class Fonts
{
public:
    void LoadFonts();
    
    ImGuiFont Small = { 12.0f };
    ImGuiFont Default = { 14.0f };
    ImGuiFont Medium = { 18.0f };
    ImGuiFont Large = { 26.0f };
    ImGuiFont ExtraLarge = { 33.0f };
};