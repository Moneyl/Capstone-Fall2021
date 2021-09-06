#include "Gui.h"
#include <imgui.h>

bool ImGuiDemoVisible = true;
bool TestWindowVisible = true;

Gui::Gui(Fonts* fonts) : _fonts(fonts)
{

}

void Gui::Update(f32 deltaTime)
{
    //Draw main menu and docking space
    DrawMainMenuBar();
    if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        DrawDockspace();

    //Draw windows
    if(TestWindowVisible)
        DrawTestWindow();

//#ifdef DEBUG_BUILD
    //The demo windows code can be found in dependencies/imgui/imgui_demo.cpp
    //There's a live web version of the demo that highlights the code as you mouse over UI elements here: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
    if(ImGuiDemoVisible)
        ImGui::ShowDemoWindow();
//#endif
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
//#ifdef DEBUG_BUILD
            if (ImGui::MenuItem("ImGui demo", "", &ImGuiDemoVisible))
            {

            }
//#endif
            if (ImGui::MenuItem("Test window", "", &TestWindowVisible))
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

void Gui::DrawTestWindow()
{
    if (!ImGui::Begin("Test window", &TestWindowVisible))
    {
        //Exit early if the window is closed
        ImGui::End();
        return;
    }

    _fonts->Large.Push();
    ImGui::Text(ICON_FA_BUG " Test window");
    _fonts->Large.Pop();
    ImGui::Separator();

    ImGui::Text("Notes:");

    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextWrapped("All windows are dockable. Click and drag the titlebar onto one of the blue zones that appear.");

    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextWrapped("See src/gui/Gui.cpp for this windows code and links to other examples.");

    ImGui::Separator();

    static bool testBool = false;
    ImGui::Checkbox("Test bool", &testBool);
    static f32 testFloat = 100.0f;
    ImGui::InputFloat("Test float", &testFloat);
    ImGui::SliderFloat("Test float slider", &testFloat, 0.0f, 1024.0f);
    
    static ImVec4 testColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    ImGui::ColorPicker4("Test color", (f32*)&testColor);

    ImGui::End();
}