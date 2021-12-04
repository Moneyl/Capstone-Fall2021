#pragma once
#include "src/Typedefs.h"
#include "gui/Fonts.h"
#include "vm/VM.h"

class Application;

class Gui
{
public:
    Gui() { }
    Gui(Application* app);

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
    //Draw message telling the user to pick a robot to see it's info. Used by robot dependent guis when none is selected.
    void DrawNoRobotWarning();
    //Draw settings window. Lists all CVars with ShowInSettings set to true.
    void DrawSettings();

    Application* _app = nullptr;
    i32 _robotIndex = -1; //Robot selected from the robot list
};