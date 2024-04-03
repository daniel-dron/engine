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

const char* init_game_window_func_name = "init_game_window";
typedef void (*init_game_window_t)(app_desc* desc);

void render_elements(u32 count) {
    GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

std::string game_logic::get_current_dll_path() const {
    return (ResourceState::get()->_workingDirectory / std::format("game{}.dll", current_dll_id)).string();
}

void game_logic::load_game(const std::string& dll_name) {
    // check if dll exists
    auto dll_path = ResourceState::get()->_workingDirectory / "build" / "testbed" / dll_name;
    KDEBUG("dll_path: {}", dll_path.string());
    assert(std::filesystem::exists(dll_path) && "DLL does not exist");

    // unload the current dll
    if (this->_dll != nullptr) {
        KDEBUG("Unloading current dll");
        this->on_shutdown();
        FreeLibrary((HINSTANCE)this->_dll);
    }

    // delete dll
    std::filesystem::path current_dll_path = this->get_current_dll_path();
    if (std::filesystem::exists(current_dll_path)) {
        KDEBUG("Deleting current dll");
        std::filesystem::remove(current_dll_path);
    }

    // create a copy of the dll with the name of the current dll (incremental)
    current_dll_path = ResourceState::get()->_workingDirectory / std::format("game{}.dll", ++current_dll_id);
    std::filesystem::copy(dll_path, current_dll_path, std::filesystem::copy_options::overwrite_existing);
    KDEBUG("Copied dll to: {}", current_dll_path.string());

    // load the dll
    HINSTANCE _dll = LoadLibrary(current_dll_path.string().c_str());
    KDEBUG("Loaded dll: {}", current_dll_path.string());
    this->_dll = _dll;

    auto default_on_init = []() -> b8 {
        std::cout << "default_on_init()" << std::endl;
        return true;
        };
    auto on_init = GetProcAddress(_dll, "on_init");
    this->on_init = on_init != nullptr ? reinterpret_cast<b8(*)(void)>(on_init) : default_on_init;

    auto default_on_update = []() -> b8 {
        std::cout << "default_on_update()" << std::endl;
        return true;
        };
    auto on_update = GetProcAddress(_dll, "on_update");
    this->on_update = on_update != nullptr ? reinterpret_cast<b8(*)(void)>(on_update) : default_on_update;

    auto default_on_render = []() -> b8 {
        std::cout << "default_on_render()" << std::endl;
        return true;
        };
    auto on_render = GetProcAddress(_dll, "on_render");
    this->on_render = on_render != nullptr ? reinterpret_cast<b8(*)(void)>(on_render) : default_on_render;

    auto default_on_resize = [](u32 width, u32 height) {
        std::cout << "default_on_resize()" << std::endl;
        };
    auto on_resize = GetProcAddress(_dll, "on_resize");
    this->on_resize = on_resize != nullptr ? reinterpret_cast<void(*)(u32, u32)>(on_resize) : default_on_resize;

    auto default_on_shutdown = []() {
        std::cout << "default_on_shutdown()" << std::endl;
        };
    auto on_shutdown = GetProcAddress(_dll, "on_shutdown");
    this->on_shutdown = on_shutdown != nullptr ? reinterpret_cast<void(*)(void)>(on_shutdown) : default_on_shutdown;

    // finally call the init function
    KDEBUG("Calling on_init");
    this->on_init();
}

std::unique_ptr<app> g_app = std::make_unique<app>();

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