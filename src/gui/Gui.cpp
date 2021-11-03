#include "Gui.h"
#include "Application.h"
#include "ImGuiExt.h"
#include <imgui.h>

bool ImGuiDemoVisible = true;
bool VariablesVisible = true;
bool StackVisible = true;
bool DisassemblerVisible = true;
bool VmStateVisible = true;
bool RobotListVisible = true;

Gui::Gui(Application* app) : _app(app)
{

}

void Gui::Update(f32 deltaTime)
{
    //Draw main menu and docking space
    DrawMainMenuBar();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        DrawDockspace();

    //Draw windows
    if (VariablesVisible)
        DrawVariables();
    if (StackVisible)
        DrawStack();
    if (DisassemblerVisible)
        DrawDisassembler();
    if (VmStateVisible)
        DrawVmState();
    if (RobotListVisible)
        DrawRobotList();

#ifdef DEBUG_BUILD
    //The demo windows code can be found in dependencies/imgui/imgui_demo.cpp
    //There's a live web version of the demo that highlights the code as you mouse over UI elements here: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
    if (ImGuiDemoVisible)
        ImGui::ShowDemoWindow();
#endif
}

void Gui::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                exit(EXIT_SUCCESS);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
#ifdef DEBUG_BUILD
            if (ImGui::MenuItem("ImGui demo", "", &ImGuiDemoVisible))
            {

            }
#endif
            if (ImGui::MenuItem(ICON_FA_SUBSCRIPT "Variables", "", &VariablesVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_TH_LIST "Stack", "", &StackVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_CODE "Disassembler", "", &DisassemblerVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_MEMORY "VM state", "", &VmStateVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_ROBOT "Robots", "", &RobotListVisible))
            {

            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Gui::DrawDockspace()
{
    //Dockspace flags
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    //Dockspace parent window flags
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImVec2 dockspaceSize = viewport->WorkSize;
    ImGui::SetNextWindowSize(dockspaceSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    //Dockspace parent window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace parent window", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    //DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGui::DockSpace(ImGui::GetID("Main dockspace"), ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

void Gui::DrawVariables()
{
    if (!ImGui::Begin(ICON_FA_SUBSCRIPT " Variables", &VariablesVisible))
    {
        //Exit early if the window is closed
        ImGui::End();
        return;
    }
    VM* vm = _app->Vm;

    _app->Fonts.Large.Push();
    ImGui::Text("Variables");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Variables table
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("VariablesTable", 2, tableFlags))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Address", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Value", ImGuiTableFlags_None);
        //Todo: Add variable names. Requires debug information stored in VmProgram
        ImGui::TableHeadersRow();

        //Fill table
        const Span<VmValue> variables = vm->Variables();
        for (const VmValue& variable : variables)
        {
            ImGui::TableNextRow();

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string((u8*)&variable - vm->Memory));

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(variable));
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void Gui::DrawStack()
{
    if (!ImGui::Begin(ICON_FA_TH_LIST " Stack", &StackVisible))
    {
        ImGui::End();
        return;
    }
    VM* vm = _app->Vm;

    _app->Fonts.Large.Push();
    ImGui::Text("Stack");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Stack table
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("StackTable", 2, tableFlags))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Address", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Value", ImGuiTableFlags_None);
        ImGui::TableHeadersRow();

        //Fill table
        for (u32 i = VM::MEMORY_SIZE; i > vm->SP; i -= sizeof(VmValue))
        {
            ImGui::TableNextRow();

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string(i));

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(*(VmValue*)&vm->Memory[i]));
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void Gui::DrawDisassembler()
{
    if (!ImGui::Begin(ICON_FA_CODE " Disassembler", &DisassemblerVisible))
    {
        ImGui::End();
        return;
    }
    VM* vm = _app->Vm;

    _app->Fonts.Large.Push();
    ImGui::Text("Disassembler");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Options
    static bool useRealOpcodeNames = false;
    if (ImGui::CollapsingHeader("Options"))
    {
        const f32 indent = 15.0f;
        ImGui::Indent(indent);

        ImGui::Checkbox("Use opcode names", &useRealOpcodeNames);
        ImGui::SameLine();
        ImGui::HelpMarker("If checked, actual opcode names are used instead of assembly instruction names. "
                          "Some assembly instructions can be converted to multiple different opcodes depending on their arguments.", _app->Fonts.Medium.GetPtr());

        ImGui::Unindent(indent);
    }

    //Draw disassembler output
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("DisassemblerTable", 2, tableFlags))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Address", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Disassembly", ImGuiTableFlags_None);
        ImGui::TableHeadersRow();

        //Fill table
        const Span<Instruction> instructions = _app->Vm->Instructions();
        for (u32 i = 0; i < instructions.size(); i++)
        {
            ImGui::TableNextRow();
            const Instruction& instruction = instructions[i];
            u32 address = (u8*)&instruction - _app->Vm->Memory; //Instruction address in VM memory

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string(address).c_str());

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(to_string(instruction, useRealOpcodeNames).c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void Gui::DrawVmState()
{
    if (!ImGui::Begin(ICON_FA_MEMORY " VM state", &VmStateVisible))
    {
        ImGui::End();
        return;
    }
    VM* vm = _app->Vm;

    _app->Fonts.Large.Push();
    ImGui::Text("VM state");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    ImGui::LabelAndValue("Memory size:", std::to_string(vm->MEMORY_SIZE) + " bytes");
    ImGui::LabelAndValue("Program size:", std::to_string(vm->InstructionsSize()) + " bytes");
    ImGui::LabelAndValue("Variables size:", std::to_string(vm->VariablesSize()) + " bytes");
    ImGui::LabelAndValue("Stack size:", std::to_string(vm->StackSize()) + "/" + std::to_string(vm->MaxStackSize()) + " bytes");

    //Draw registers in table
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("RegistersTable", 2, tableFlags))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Register", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Value", ImGuiTableFlags_None);
        ImGui::TableHeadersRow();

        //Fill table
        for (u32 i = 0; i < VM::NUM_REGISTERS; i++)
        {
            ImGui::TableNextRow();
            
            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("r" + std::to_string(i));

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(vm->Registers[i]));
        }

        //Special registers
        //PC
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("PC");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(std::to_string(vm->PC));

        //SP
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("SP");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(std::to_string(vm->SP));

        ImGui::EndTable();
    }

    ImGui::End();
}

void Gui::DrawRobotList()
{
    if (!ImGui::Begin(ICON_FA_ROBOT " Robots", &RobotListVisible))
    {
        ImGui::End();
        return;
    }
    VM* vm = _app->Vm;

    _app->Fonts.Large.Push();
    ImGui::Text("Robots");
    _app->Fonts.Large.Pop();
    ImGui::Separator();


    ImGui::End();
}