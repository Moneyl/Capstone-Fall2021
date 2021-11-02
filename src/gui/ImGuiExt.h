#pragma once
#include "Typedefs.h"
#include <string_view>
#include <imgui.h>
#include <string>

//Contains overrides and helper functions for dear imgui
namespace ImGui
{
    IMGUI_API void Text(const std::string& text);
    IMGUI_API void TextColored(const std::string& text, const ImVec4& color);
    IMGUI_API void TextUnformatted(const std::string& text);
    IMGUI_API void TextUnformatted(std::string_view text);

    //Draws (?) with a mouseover tooltip. Useful to place next to other UI elements for explanation.
    void HelpMarker(const std::string& tooltip, ImFont* font = nullptr);
}