#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <cassert>

#include "glad/glad.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "renderer/resources/bindable.hpp"
#include "entry.h"

const char* init_game_window_func_name = "init_game_window";
typedef void (*init_game_window_t)(app_desc* desc);

void load_game_funcs(HINSTANCE dll, app_behaviour* app) {
    auto default_on_init = []() -> b8 {
        std::cout << "default_on_init()" << std::endl;
        return true;
        };
    auto on_init = GetProcAddress(dll, "on_init");
    app->on_init = on_init != nullptr ? reinterpret_cast<b8(*)(void)>(on_init) : default_on_init;

    auto default_on_update = []() -> b8 {
        std::cout << "default_on_update()" << std::endl;
        return true;
        };
    auto on_update = GetProcAddress(dll, "on_update");
    app->on_update = on_update != nullptr ? reinterpret_cast<b8(*)(void)>(on_update) : default_on_update;

    auto default_on_render = []() -> b8 {
        std::cout << "default_on_render()" << std::endl;
        return true;
        };
    auto on_render = GetProcAddress(dll, "on_render");
    app->on_render = on_render != nullptr ? reinterpret_cast<b8(*)(void)>(on_render) : default_on_render;

    auto default_on_resize = [](u32 width, u32 height) {
        std::cout << "default_on_resize()" << std::endl;
        };
    auto on_resize = GetProcAddress(dll, "on_resize");
    app->on_resize = on_resize != nullptr ? reinterpret_cast<void(*)(u32, u32)>(on_resize) : default_on_resize;

    auto default_on_shutdown = []() {
        std::cout << "default_on_shutdown()" << std::endl;
        };
    auto on_shutdown = GetProcAddress(dll, "on_shutdown");
    app->on_shutdown = on_shutdown != nullptr ? reinterpret_cast<void(*)(void)>(on_shutdown) : default_on_shutdown;
}

std::unique_ptr<app_behaviour> app;

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    app->on_resize(width, height);
}

int main() {
    HINSTANCE gameDLL = LoadLibrary("../testbed/testbedd.dll");
    assert(gameDLL && "no game dll!");

    auto init_game = (init_game_window_t)GetProcAddress(gameDLL, init_game_window_func_name);
    assert(init_game && "no init game!");

    auto desc = std::make_unique<app_desc>();
    init_game(desc.get());

    app = std::make_unique<app_behaviour>();
    load_game_funcs(gameDLL, app.get());

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(0);

    GLFWwindow* window = glfwCreateWindow(desc->width, desc->height, desc->window_name.c_str(), nullptr, nullptr);
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

    app->on_init();
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