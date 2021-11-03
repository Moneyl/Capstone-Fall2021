#pragma once
#include "Typedefs.h"
#include <string_view>
#include <imgui.h>
#include <string>

//Contains overrides and helper functions for dear imgui
namespace ImGui
{
    const ImVec4 SecondaryColor = { 0.32f, 0.67f, 1.0f, 1.0f }; //Light blue. Used by LabelAndValue() for value text.

    IMGUI_API void Text(const std::string& text);
    IMGUI_API void TextColored(const std::string& text, const ImVec4& color);
    IMGUI_API void TextUnformatted(const std::string& text);
    IMGUI_API void TextUnformatted(std::string_view text);

    //Draws (?) with a mouseover tooltip. Useful to place next to other UI elements for explanation.
    void HelpMarker(const std::string& tooltip, ImFont* font = nullptr);

    //Draws a label and value in one line. E.g. `Label: value`. Separate colors are used for the label and value.
    void LabelAndValue(std::string_view label, std::string_view value);
}