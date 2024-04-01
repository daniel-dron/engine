#include <stdio.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <Windows.h>

#include "engine.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

typedef void (*init_game_t)();

void engine_api() {
    printf("game called engine!\n");
}

int main()
{
    HINSTANCE gameDLL = LoadLibrary("../testbed/testbed.dll");
    assert(gameDLL && "no game dll!");

    init_game_t init_game = (init_game_t)GetProcAddress(gameDLL, "init_game");
    assert(init_game && "no init game!");

    init_game();

    Assimp::Importer importer;
    const auto scene = importer.ReadFile("ehello", aiProcess_Triangulate);

    glm::vec3 t = glm::vec3(3.6f);
    printf("%f", t.x);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(0);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "kldjasiodnais", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::exception();
    }

    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 430");
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwGetCurrentContext();

        glClearColor(0.4f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        ImGui::ShowDemoWindow();

        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            auto backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);

            glfwSwapBuffers(glfwGetCurrentContext());
        }
        glfwPollEvents();
    }

    return 1;
}