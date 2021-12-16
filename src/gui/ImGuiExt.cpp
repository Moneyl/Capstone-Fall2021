#include "ImGuiExt.h"

struct InputTextCallback_UserData
{
    std::string* Str;
    ImGuiInputTextCallback  ChainCallback;
    void* ChainCallbackUserData;
};

//Callback used by ImGui::InputText() overloads to resize string buffer as necessary
static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        //Resize buffer
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        //Call next callback in chain, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

namespace ImGui
{
    IMGUI_API bool ImGui::InputText(const std::string& label, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = &str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return InputText(label.c_str(), (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cb_user_data);
    }

    IMGUI_API void Text(const std::string& text)
    {
        ImGui::Text(text.c_str());
    }

    IMGUI_API void TextColored(const std::string& text, const ImVec4& color)
    {
        ImGui::TextColored(color, text.c_str());
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

