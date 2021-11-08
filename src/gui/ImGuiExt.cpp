#include "ImGuiExt.h"

namespace ImGui
{
    IMGUI_API void Text(const std::string& text)
    {
        ImGui::Text(text.c_str());
    }

    IMGUI_API void TextColored(const std::string& text, const ImVec4& color)
    {
        ImGui::TextColored(text.c_str(), color);
    }

    IMGUI_API void TextUnformatted(const std::string& text)
    {
        ImGui::TextUnformatted(text.data(), text.data() + text.size());
    }

    IMGUI_API void TextUnformatted(std::string_view text)
    {
        ImGui::TextUnformatted(text.data(), text.data() + text.length());
    }

    void TooltipOnPrevious(const std::string& tooltip, ImFont* font)
    {
        if (ImGui::IsItemHovered()) //Is previous UI element being mouse hovered?
        {
            if (!font)
                font = ImGui::GetIO().FontDefault;

            ImGui::PushFont(font);
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f); //Wrap tooltip at ~35 characters
            ImGui::TextUnformatted(tooltip);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
            ImGui::PopFont();
        }
    }

    void HelpMarker(const std::string& tooltip, ImFont* font)
    {
        ImGui::TextDisabled("(?)");
        ImGui::TooltipOnPrevious(tooltip, font);
    }

    void LabelAndValue(std::string_view label, std::string_view value)
    {
        //Draw label
        ImGui::TextUnformatted(label);
        
        //Draw value on same line with secondary color
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::SecondaryColor);
        ImGui::TextUnformatted(value);
        ImGui::PopStyleColor();
    }
}

