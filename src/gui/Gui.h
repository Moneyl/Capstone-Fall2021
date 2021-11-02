#pragma once
#include "src/Typedefs.h"
#include "gui/Fonts.h"
#include "vm/VM.h"

class Application;

class Gui
{
public:
    Gui() { }
    Gui(Application* fonts);

    void Update(f32 deltaTime);

private:
    //Draw main menu bar at the top of the window
    void DrawMainMenuBar();
    //Draw dockspace over the rest of the window that other windows can be docked to
    void DrawDockspace();
    //List of variables used by the selected VMs program and their values
    void DrawVariables();
    //VM stack state
    void DrawStack();
    //Disassembly of the selected VMs program
    void DrawDisassembler();
    //VM state such as register values and memory size
    void DrawVmState();
    //List of robots. When selected the other UI panels will display that robots state.
    void DrawRobotList();

    Application* _app = nullptr;
    VM* _vm = nullptr;
};