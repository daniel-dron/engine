#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <cassert>
#include <thread>

#include "glad/glad.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "renderer/resources/bindable.hpp"
#include "renderer/resources/resources.hpp"
#include "entry.h"
#include "renderer/resources/gl_errors.hpp"
#include "app.hpp"

void render_elements(u32 count) {
    GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    g_app->logic->on_resize(width, height);
}

int main(int argc, char* argv[]) {
    ResourceState::get()->init(argv[0]);

    {
        auto desc = std::make_unique<app_desc>();
        desc->pos_x = 100;
        desc->pos_y = 100;
        desc->width = 1920;
        desc->height = 1080;
        desc->window_name = "default window name";
        g_app->desc = std::move(desc);
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(0);

    GLFWwindow* window = glfwCreateWindow(g_app->desc->width, g_app->desc->height, g_app->desc->window_name.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::exception();
    }

    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 430");
    }

    glfwSetWindowSizeCallback(window, windowSizeCallback);

    g_app->logic = std::make_unique<game_logic>();
    g_app->logic->load_game("testbedd.dll");
    auto& app = g_app->logic;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetCurrentContext();

        app->on_update();

        glClearColor(0.4f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        app->on_render();
        ImGui::ShowDemoWindow();

        {
            ImGui::Begin("dll");
            if (ImGui::Button("Reload DLL")) {
                app->load_game("testbedd.dll");
            }
            ImGui::End();
        }

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

    app->on_shutdown();

    return 1;
}