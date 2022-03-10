#include "Editor.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


static bool dockspaceOpen = true;
static int selectedEntityID = -1;
static bool updateAllShapes = true;


void initEditor(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
}


void drawEditor(RayMarchingManager& rayMarching)
{
    //New Frame
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    //New Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool demo = false;
    if (demo)
    {
        ImGui::ShowDemoWindow(&demo);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return;
    }

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSize = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }


    // Settings
    if (ImGui::Begin("Settings"))
    {
         ImGui::Text("Fps %.1f", ImGui::GetIO().Framerate);
         ImGui::Text("Samples : %d / 20", rayMarching.getCurrentSample());


        if (ImGui::CollapsingHeader("Camera"))
        {
            if(ImGui::DragFloat3("Camera", &rayMarching.getCamera()._eye[0], 0.1f, -10.0f, 10.0f))
            {
                rayMarching.UpdateView();
                rayMarching.getCamera().updateCamera();
            }

        }


        if (ImGui::CollapsingHeader("World Outliner", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int shapesCount = 0;
            for (const auto& shape : rayMarching.getShapes())
            {
                if (ImGui::Selectable(shape.name.c_str()))
                {
                    selectedEntityID = shapesCount;
                }

                shapesCount++;
                if (shapesCount > 10)
                {
                    break;
                }
            }
        }

        if (selectedEntityID >= 0 && selectedEntityID < rayMarching.getNumShapes())
        {
            if (ImGui::CollapsingHeader(std::string("Shape - " + rayMarching.getShapeAtIndex(selectedEntityID).name).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::DragFloat3("Location", &rayMarching.getShapeAtIndex(selectedEntityID).position[0], 0.1f, -10.0f, 10.0f))
                {
                    rayMarching.UpdateScene();
                }
                if (ImGui::DragFloat3("Scale", &rayMarching.getShapeAtIndex(selectedEntityID).size[0], 0.1f, -10.0f, 10.0f))
                {
                    rayMarching.UpdateScene();
                }

                if (ImGui::TreeNodeEx("Ray Marching", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Update all shapes", &updateAllShapes);

                    if (ImGui::Combo("Operation", &(int&)rayMarching.getShapeAtIndex(selectedEntityID).operation, "Default\0Blend\0\0"))
                    {
                        if (updateAllShapes)
                        {
                            for (auto& shape : rayMarching.getShapes())
                            {
                                shape.operation = rayMarching.getShapeAtIndex(selectedEntityID).operation;
                            }
                        }
                        rayMarching.UpdateScene();
                    }

                    if (rayMarching.getShapeAtIndex(selectedEntityID).operation == EOperation::BLEND
                        && ImGui::DragFloat("Blend Strength", &rayMarching.getShapeAtIndex(selectedEntityID).blendStrength, 0.01f, 0.0f, 1.0f))
                    {
                        if (updateAllShapes)
                        {
                            for (auto& shape : rayMarching.getShapes())
                            {
                                shape.blendStrength = rayMarching.getShapeAtIndex(selectedEntityID).blendStrength;
                            }
                        }
                        rayMarching.UpdateScene();
                    }

                    ImGui::TreePop();
                }
            }
        }
    }
    ImGui::End(); // Settings



    // 3D Viewer
    if (ImGui::Begin("Viewer 3D"))
    {
        //ImVec2 wsize = ImGui::GetContentRegionAvail();
        ImVec2 wsize = { 600, 480 };

        ImGui::Image((ImTextureID)rayMarching.getFbo().getTextureId(), wsize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();


    ImGui::End(); // Main Window
}



void renderEditor()
{
    // Render ImGUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}