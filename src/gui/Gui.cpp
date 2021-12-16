#include "Gui.h"
#include "Application.h"
#include "ImGuiExt.h"
#include "Config.h"
#include "utility/Filesystem.h"
#include <imgui.h>

CVar CVar_UIScale("UI Scale", ConfigType::Float,
    "Scale of the user interface.",
    ConfigValue(1.0f), //Default value
    true,  //ShowInSettings
    false, //IsFolderPath
    false, //IsFilePath
    0.5f, 2.0f //Min-max
);

bool ImGuiDemoVisible = true;
bool VariablesVisible = true;
bool StackVisible = true;
bool DisassemblerVisible = true;
bool VmStateVisible = true;
bool RobotListVisible = true;
bool SettingsVisible = false;
bool ArenaGuiVisible = true;

Gui::Gui(Application* app) : _app(app)
{
    if (app->Arena.Robots.size() > 0)
        _robotIndex = 0;
}

void Gui::Update(f32 deltaTime)
{
    //Update UI scale if it changed since last frame
    static f32 lastFrameScale = 1.0f;
    if (lastFrameScale != CVar_UIScale.Get<f32>())
    {
        lastFrameScale = CVar_UIScale.Get<f32>();
        ImGui::GetIO().FontGlobalScale = lastFrameScale;
    }

    //Draw main menu and docking space
    DrawMainMenuBar();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        DrawDockspace();

#ifdef DEBUG_BUILD
    //The demo windows code can be found in dependencies/imgui/imgui_demo.cpp
    //There's a live web version of the demo that highlights the code as you mouse over UI elements here: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
    if (ImGuiDemoVisible)
        ImGui::ShowDemoWindow();
#endif

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
    if (SettingsVisible)
        DrawSettings();
    if (ArenaGuiVisible)
        DrawArenaGui();
    if (TournamentCreatorVisible)
        DrawTournamentPopup();

    DrawTournamentStats();
}

void Gui::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Settings", nullptr, &SettingsVisible))
            {

            }
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
            if (ImGui::MenuItem(ICON_FA_SUBSCRIPT " Variables", "", &VariablesVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_TH_LIST " Stack", "", &StackVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_CODE " Disassembler", "", &DisassemblerVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_MEMORY " VM state", "", &VmStateVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_ROBOT " Robots", "", &RobotListVisible))
            {

            }
            if (ImGui::MenuItem(ICON_FA_UNIVERSITY " Arena", "", &ArenaGuiVisible))
            {

            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Game"))
        {
            ImGui::MenuItem(ICON_FA_SYNC_ALT " Auto reload robots", "", &_app->Arena.RobotAutoReloadEnabled);
            ImGui::TooltipOnPrevious("Auto recompile robot programs when their source file is edited and saved. This also resets the VM (memory, registers, stack, variables, etc).");

            ImGui::MenuItem(ICON_FA_SITEMAP " Start tournament", "F2", &TournamentCreatorVisible);
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

    _app->Fonts.Large.Push();
    ImGui::Text("Variables");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Don't draw gui if no robot is selected
    if (_robotIndex == -1 || _robotIndex >= _app->Arena.Robots.size())
    {
        DrawNoRobotWarning();
        ImGui::End();
        return;
    }
    Robot* robot = _app->Arena.Robots[_robotIndex];

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
        const Span<VmValue> variables = robot->Vm->Variables();
        for (const VmValue& variable : variables)
        {
            ImGui::TableNextRow();

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string((u8*)&variable - robot->Vm->Memory));

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

    _app->Fonts.Large.Push();
    ImGui::Text("Stack");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Don't draw gui if no robot is selected
    if (_robotIndex == -1 || _robotIndex >= _app->Arena.Robots.size())
    {
        DrawNoRobotWarning();
        ImGui::End();
        return;
    }
    Robot* robot = _app->Arena.Robots[_robotIndex];

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
        for (u32 i = VM::MEMORY_SIZE - sizeof(VmValue); i > robot->Vm->SP; i -= sizeof(VmValue))
        {
            ImGui::TableNextRow();

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string(i));

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(*(VmValue*)&robot->Vm->Memory[i]));
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

    _app->Fonts.Large.Push();
    ImGui::Text("Disassembler");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Don't draw gui if no robot is selected
    if (_robotIndex == -1 || _robotIndex >= _app->Arena.Robots.size())
    {
        DrawNoRobotWarning();
        ImGui::End();
        return;
    }
    Robot* robot = _app->Arena.Robots[_robotIndex];

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
    if (ImGui::BeginTable("DisassemblerTable", 3, tableFlags))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Address", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Disassembly", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Cycles", ImGuiTableFlags_None);
        ImGui::TableHeadersRow();

        //Fill table
        const Span<Instruction> instructions = robot->Vm->Instructions();
        for (u32 i = 0; i < instructions.size(); i++)
        {
            ImGui::TableNextRow();
            const Instruction& instruction = instructions[i];
            u32 address = (u8*)&instruction - robot->Vm->Memory; //Instruction address in VM memory

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(std::to_string(address).c_str());

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(to_string(instruction, useRealOpcodeNames).c_str());

            //Column 2
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(std::to_string(robot->Vm->GetInstructionDuration(instruction)).c_str());
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

    _app->Fonts.Large.Push();
    ImGui::Text("VM state");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Don't draw gui if no robot is selected
    if (_robotIndex == -1 || _robotIndex >= _app->Arena.Robots.size())
    {
        DrawNoRobotWarning();
        ImGui::End();
        return;
    }
    Robot* robot = _app->Arena.Robots[_robotIndex];

    const u32 min = 0;
    const u32 max = 1000;
    ImGui::SliderScalar("Cycles/second", ImGuiDataType_U32, &_app->Arena.CyclesPerSecond, &min, &max);
    ImGui::LabelAndValue("Memory size:", std::to_string(robot->Vm->MEMORY_SIZE) + " bytes");
    ImGui::LabelAndValue("Program size:", std::to_string(robot->Vm->InstructionsSize()) + " bytes");
    ImGui::LabelAndValue("Variables size:", std::to_string(robot->Vm->VariablesSize()) + " bytes");
    ImGui::LabelAndValue("Stack size:", std::to_string(robot->Vm->StackSize()) + "/" + std::to_string(robot->Vm->MaxStackSize()) + " bytes");

    ImGui::Separator();
    _app->Fonts.Medium.Push();
    ImGui::Text("Registers");
    _app->Fonts.Medium.Pop();
    ImGui::Separator();

    //Draw registers in table
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("RegistersTable", 2, tableFlags, { 0.0f, 200.0f }))
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
            ImGui::Text(std::to_string(robot->Vm->Registers[i]));
        }

        //Special registers
        //PC
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("PC");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(std::to_string(robot->Vm->PC));

        //SP
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("SP");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(std::to_string(robot->Vm->SP));

        ImGui::EndTable();
    }

    ImGui::Separator();
    _app->Fonts.Medium.Push();
    ImGui::Text("Config");
    _app->Fonts.Medium.Pop();
    ImGui::Separator();

    if (ImGui::BeginTable("ConfigTable", 2, tableFlags, { 0.0f, 250.0f }))
    {
        //Setup columns
        ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
        ImGui::TableSetupColumn("Name", ImGuiTableFlags_None);
        ImGui::TableSetupColumn("Value", ImGuiTableFlags_None);
        ImGui::TableHeadersRow();

        //Fill table
        for (VmConfig& config : robot->Vm->Config)
        {
            ImGui::TableNextRow();

            //Column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(config.Name.c_str());

            //Column 1
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(config.Value));
        }

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

    _app->Fonts.Large.Push();
    ImGui::Text("Robots");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    //Disable borders and change background color for robot list
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_ChildBg]);

    //Draw robot list
    if (ImGui::BeginListBox("##RobotsList", { -FLT_MIN, -FLT_MIN }))
    {
        for (u32 i = 0; i < _app->Arena.Robots.size(); i++)
        {
            Robot* robot = _app->Arena.Robots[i];
            std::string label = "Robot " + std::to_string(i) + " - " + std::filesystem::path(robot->SourcePath()).filename().string();
            bool selected = (i == _robotIndex);

            //Draw selectable box for robot
            ImVec2 cursorPos = ImGui::GetCursorPos();
            if (ImGui::Selectable(label.c_str(), &selected, 0, { ImGui::GetWindowWidth(), 75.0f }))
            {
                _robotIndex = i;
            }
            ImVec2 endCursorPos = ImGui::GetCursorPos();

            //Draw stats within selectable box
            ImVec2 progressBarSize = { -FLT_MIN, 20.0f };
            cursorPos.y += ImGui::GetTextLineHeight();
            cursorPos.y += ImGui::GetStyle().ItemSpacing.y;
            ImGui::SetCursorPos(cursorPos);
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            //Health
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, { 0.259f, 0.6f, 0.114f, 1.0f });
            ImGui::ProgressBar(robot->Armor / robot->MaxArmor, progressBarSize, "Armor");
            ImGui::PopStyleColor();

            //Heat
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, { 0.6f, 0.145f, 0.114f, 1.0f });
            ImGui::ProgressBar(robot->Heat / robot->MaxHeat, progressBarSize, "Heat");
            ImGui::PopStyleColor();

            //Set cursor to end of the selectable
            ImGui::SetCursorPos(endCursorPos);
            if (i != _app->Arena.Robots.size() - 1)
                ImGui::Separator();
        }

        ImGui::EndListBox();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::End();
}

void Gui::DrawNoRobotWarning()
{
    ImGui::TextWrapped(ICON_FA_EXCLAMATION_CIRCLE " Select a robot from the robots list. You can re-open closed windows from 'View' on the main menu bar.");
}

void Gui::DrawSettings()
{
    if (!SettingsVisible)
        return;

    const std::string title = "Settings";
    ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopup(title.c_str(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
    {
        Fonts* fonts = &_app->Fonts;
        fonts->Large.Push();
        ImGui::Text(ICON_FA_COG " Settings");
        fonts->Large.Pop();
        ImGui::Separator();

        //Draw settings
        Config* config = Config::Get();
        for (CVar* var : CVar::Instances)
        {
            //Only draw settings with <ShowInSettings>True</ShowInSettings>
            if (!var->ShowInSettings)
                continue;

            auto drawTooltip = [&]()
            {
                ImGui::TooltipOnPrevious(var->Description, fonts->Default.GetPtr());
            };

            switch (var->Type)
            {
            case ConfigType::Int:
                if (var->Min && var->Max) //User slider if min-max is provided
                {
                    i32 min = (i32)var->Min.value();
                    i32 max = (i32)var->Max.value();
                    if (ImGui::SliderInt(var->Name.c_str(), &var->Get<i32>(), min, max))
                        config->Save();
                }
                else
                {
                    if (ImGui::InputInt(var->Name.c_str(), &var->Get<i32>()))
                        config->Save();

                }
                drawTooltip();
                break;
            case ConfigType::Uint:
                if (var->Min && var->Max) //User slider if min-max is provided
                {
                    u32 min = (u32)var->Min.value();
                    u32 max = (u32)var->Max.value();
                    if (ImGui::SliderScalar(var->Name.c_str(), ImGuiDataType_U32, (void*)&var->Get<u32>(), (void*)&min, (void*)&max))
                        config->Save();
                }
                else
                {
                    if (ImGui::InputScalar(var->Name.c_str(), ImGuiDataType_U32, &var->Get<u32>()))
                        config->Save();
                }
                drawTooltip();
                break;
            case ConfigType::Float:
                if (var->Min && var->Max) //User slider if min-max is provided
                {
                    if (ImGui::SliderFloat(var->Name.c_str(), (f32*)&var->Get<f32>(), var->Min.value(), var->Max.value()))
                        config->Save();
                }
                else
                {
                    if (ImGui::InputFloat(var->Name.c_str(), &var->Get<f32>()))
                        config->Save();
                }
                drawTooltip();
                break;
            case ConfigType::Bool:
                if (ImGui::Checkbox(var->Name.c_str(), &var->Get<bool>()))
                    config->Save();

                drawTooltip();
                break;
            case ConfigType::String:
                if (var->IsFilePath)
                {
                    ImGui::LabelAndValue(var->Name, var->Get<std::string>());
                    drawTooltip();
                    ImGui::SameLine();
                    if (ImGui::Button("Browse..."))
                    {
                        std::optional<std::string> output = OpenFile();
                        if (output)
                        {
                            var->Value = output.value();
                            config->Save();
                        }
                    }
                }
                else if (var->IsFolderPath)
                {
                    ImGui::LabelAndValue(var->Name, var->Get<std::string>());
                    drawTooltip();
                    ImGui::SameLine();
                    if (ImGui::Button("Browse..."))
                    {
                        std::optional<std::string> output = OpenFolder();
                        if (output)
                        {
                            var->Value = output.value();
                            config->Save();
                        }
                    }
                }
                else
                {
                    if (ImGui::InputText(var->Name, var->Get<std::string>()))
                        config->Save();

                    drawTooltip();
                }
                break;
            case ConfigType::List:
                if (ImGui::BeginListBox(var->Name.c_str()))
                {
                    drawTooltip();
                    std::vector<std::string> & values = var->Get<std::vector<std::string>>();
                    for (std::string& value : values)
                        if (ImGui::InputText("##ListBoxItem" + std::to_string((u64)var), value))
                            config->Save();

                    ImGui::EndListBox();
                }
                break;
            case ConfigType::Invalid:
            default:
                break;
            }
        }

        if (ImGui::Button("Close"))
        {
            SettingsVisible = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Gui::DrawTournamentPopup()
{
    const std::string popupTitle = "Start tournament";
    static std::vector<std::string> robotOptions = {}; //All robots in the folder at the time of opening the popup
    static std::vector<std::string> robotList = {}; //Robots chosen to be in the tournament
    static u32 numStages = 3;

    //Open and draw popup
    if (!ImGui::IsPopupOpen(popupTitle.c_str()))
    {
        ImGui::OpenPopup(popupTitle.c_str());

        //Fill robotOptions with files in the /robots folder
        robotOptions.clear();
        for (auto& entry : std::filesystem::directory_iterator(BuildConfig::RobotFolderPath))
            if (entry.is_regular_file() && entry.path().extension() == ".sunyat")
                robotOptions.push_back(entry.path().filename().replace_extension("").string());

        //Reset robot list & pick a few random one
        robotList.clear();
        for (size_t i = 0; i < 4; i++)
        {
            std::string newBot = robotOptions[rand() % robotOptions.size()];
            robotList.push_back(newBot);
        }
    }
    if (ImGui::BeginPopupModal(popupTitle.c_str(), &TournamentCreatorVisible))//, ImGuiWindowFlags_AlwaysAutoResize))
    {
        _app->Fonts.Large.Push();
        ImGui::Text(ICON_FA_COG " Settings");
        _app->Fonts.Large.Pop();
        ImGui::Separator();

        ImGui::InputScalar("# of stages", ImGuiDataType_U32, &numStages);

        _app->Fonts.Large.Push();
        ImGui::Text(ICON_FA_ROBOT " Robots");
        _app->Fonts.Large.Pop();
        ImGui::Separator();

        //Add random bot
        if (ImGui::Button(ICON_FA_PLUS_SQUARE))
        {
            std::string newBot = robotOptions[rand() % robotOptions.size()];
            robotList.push_back(robotOptions[rand() % robotOptions.size()]);
        }

        //Draw robot list
        if (ImGui::BeginListBox("##RobotsList", { -FLT_MIN, 250 }))
        {
            //Draw combo selector for each robot
            size_t i = 0;
            size_t removeIndex = ULLONG_MAX;
            for (std::string& robot : robotList)
            {
                const std::string id = "##" + std::to_string(i); //Must include index in each combo so they have a unique ID
                ImGui::PushID(id.c_str());
                if (ImGui::BeginCombo("##RobotSelector", robot.c_str()))
                {
                    for (const std::string& option : robotOptions)
                    {
                        bool selected = (robot == option);
                        if (ImGui::Selectable(option.c_str(), &selected))
                            robot = option;
                    }

                    ImGui::EndCombo();
                }
                
                //Robot remove button
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_MINUS_SQUARE))
                    removeIndex = i; //Removed outside of the loop if removeIndex changed
                
                ImGui::PopID();
                i++;
            }

            //Remove robot
            if (removeIndex != ULLONG_MAX)
                robotList.erase(robotList.begin() + removeIndex);

            ImGui::EndListBox();
        }

        if (ImGui::Button("Cancel"))
        {
            TournamentCreatorVisible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Start"))
        {
            //Todo: Replace with StartTournment function that handles resetting scores
            _app->Arena.Scores.clear();
            for (const std::string& name : robotList)
            {
                std::string path = BuildConfig::RobotFolderPath + name + ".sunyat";
                _app->Arena.Scores[path] = 0;
            }
            _app->Arena.Reset(robotList);
            _app->Arena.NumStages = numStages;
            _app->Arena.State = ArenaState::Tournament;
            _app->Arena.Stage = 0;
            TournamentCreatorVisible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Gui::DrawTournamentStats()
{
    Arena& arena = _app->Arena;
    bool visible = arena.State == ArenaState::TournamentStageDone || arena.State == ArenaState::TournamentComplete;
    std::string title = "Tournament stats";

    if (visible)
        ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopupModal(title.c_str()))
    {
        _app->Fonts.Large.Push();
        ImGui::Text(ICON_FA_CHART_LINE " Stats");
        _app->Fonts.Large.Pop();
        ImGui::Separator();

        //Stats table
        ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("RegistersTable", 2, tableFlags, { 0.0f, 200.0f }))
        {
            //Setup columns
            ImGui::TableSetupScrollFreeze(0, 1); //Make header row always visible when scrolling
            ImGui::TableSetupColumn("Robot", ImGuiTableFlags_None);
            ImGui::TableSetupColumn("Score", ImGuiTableFlags_None);
            ImGui::TableHeadersRow();

            //Fill table
            for (const auto& kv : arena.Scores)
            {
                const std::string& path = kv.first;
                const u32 score = kv.second;
                ImGui::TableNextRow();

                //Column 0
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(std::filesystem::path(path).replace_extension("").filename().string().c_str());

                //Column 1
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(std::to_string(score).c_str());
            }

            ImGui::EndTable();
        }

        //Overall status
        _app->Fonts.Medium.Push();
        if (arena.State == ArenaState::TournamentStageDone)
        {
            ImGui::TextColored(arena.Winner + " is winning so far!", ImGui::SecondaryColor);
            ImGui::TextColored("Stage complete!", ImGui::SecondaryColor);
        }
        if (arena.State == ArenaState::TournamentComplete)
        {
            //Find highest score
            u32 max = 0;
            std::string winnerName = "";
            for(auto& kv : arena.Scores)
                if (kv.second > max)
                {
                    max = kv.second;
                    winnerName = std::filesystem::path(kv.first).replace_extension("").filename().string();
                }

            if (winnerName == "")
                ImGui::TextColored("No winner. All scores the same.", ImGui::SecondaryColor);
            else
                ImGui::TextColored(winnerName + " is the winner!", ImGui::SecondaryColor);

            ImGui::TextColored("Tournament complete!", ImGui::SecondaryColor);
        }
        _app->Fonts.Medium.Pop();

        if (arena.State == ArenaState::TournamentStageDone)
        {
            if (ImGui::Button("Continue tournament"))
            {
                arena.State = ArenaState::Tournament;
                arena.GameSpeed = 1.0f;
                arena.Reset();
                ImGui::CloseCurrentPopup();
            }
        }
        else if (arena.State == ArenaState::TournamentComplete)
        {
            if (ImGui::Button("Done"))
            {
                arena.State = ArenaState::Normal;
                arena.GameSpeed = 1.0f;
                arena.Stage = 0;
                ImGui::CloseCurrentPopup();
            }
        }


        ImGui::EndPopup();
    }
}

void Gui::DrawArenaGui()
{
    if (!ImGui::Begin(ICON_FA_UNIVERSITY " Arena", &ArenaGuiVisible))
    {
        //Exit early if the window is closed
        ImGui::End();
        return;
    }

    _app->Fonts.Large.Push();
    ImGui::Text("Arena");
    _app->Fonts.Large.Pop();
    ImGui::Separator();

    ImGui::SliderFloat("Game speed", &_app->Arena.GameSpeed, 0.0f, Arena::MaxGameSpeed);
    ImGui::TooltipOnPrevious("Game speed is multiplied by this. E.g. 2.0 would be two times quicker, 0.5 would be half as quick.");

    ImGui::End();
}