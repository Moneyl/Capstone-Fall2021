#include "Fonts.h"

void Fonts::LoadFonts()
{
    //Define ranges for merging main font with image font. This will let us use icons by passing ICON_FA macros to ImGui::Text()
    static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 }; //Note: This must stay alive the duration of font usage. So it's static.
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.PixelSnapH = true;

    //Load fonts
    ImGuiIO& io = ImGui::GetIO();
    Small.Load(io, &iconConfig, iconRanges);
    Default.Load(io, &iconConfig, iconRanges);
    Medium.Load(io, &iconConfig, iconRanges);
    Large.Load(io, &iconConfig, iconRanges);
    ExtraLarge.Load(io, &iconConfig, iconRanges);

    io.FontDefault = Default.GetPtr();
}