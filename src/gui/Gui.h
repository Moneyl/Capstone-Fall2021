#pragma once
#include "src/Typedefs.h"
#include "gui/Fonts.h"

class Gui
{
public:
    Gui() { }
    Gui(Fonts* fonts);

    void Update(f32 deltaTime);

private:
    //Draw main menu bar at the top of the window
    void DrawMainMenuBar();
    //Draw dockspace over the rest of the window that other windows can be docked to
    void DrawDockspace();
    //Draw window for testing out imgui
    void DrawTestWindow();

    Fonts* _fonts = nullptr;
};