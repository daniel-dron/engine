#include "app.hpp"

#include "defines.hpp"
#include <renderer/resources/resources.hpp>
#include <Windows.h>
#include <iostream>
#include "glad/glad.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

std::unique_ptr<app> g_app;

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
}

std::unique_ptr<app> app::create(std::unique_ptr<app_desc> desc)
{
    auto _app = std::make_unique<app>();
    _app->_desc = std::move(desc);
    return _app;
}

void app::add_logic(const std::string& dll_name)
{
    _logic = std::make_unique<game_logic>();
    _logic->load_game("testbedd.dll");
}

b8 app::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(1);

    _window = glfwCreateWindow(_desc->width, _desc->height, _desc->window_name.c_str(), nullptr, nullptr);
    if (_window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::exception();
    }

    glfwMakeContextCurrent(_window);

    // set callbacks
    glfwSetWindowSizeCallback(_window, _window_size_callback);
    glfwSetCursorPosCallback(_window, _cursor_callback);
    glfwSetMouseButtonCallback(_window, _mouse_callback);
    glfwSetKeyCallback(_window, _key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::exception();
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    return _logic->on_init();

    return true;
}

void app::run()
{
    init();

    while (!glfwWindowShouldClose(_window))
    {
        glfwGetCurrentContext();

        update();

        {
            glClearColor(0.4f, 0.0f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        render();

        // end frame
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

        // clear just-pressed keys
        this->clear();
    }

    _logic->on_shutdown();
}

b8 app::update()
{
    // calculate delta time
    const auto now = this->now();
    _delta = float(now - _last_frame) / NS_PER_SECOND;
    _last_frame = now;

    // call game logic update
    _logic->on_update();

    if (keys[GLFW_KEY_ESCAPE].down) {
        glfwSetWindowShouldClose(_window, true);
    }

    return true;
}

b8 app::render()
{
    ImGui::ShowDemoWindow();

    {
        ImGui::Begin("dll");
        if (ImGui::Button("Reload DLL")) {
            if (_logic) {
                _logic->load_game("testbedd.dll");

                // call on_init after loading new dll
                _logic->on_init();
            }
        }
        ImGui::End();
    }

    {
        ImGui::Begin("DEBUG");
        ImGui::Text("FPS: %.1f", 1.0f / _delta);
        ImGui::Text("Mouse Pos: %.1f, %.1f", mouse_pos.x, mouse_pos.y);
        ImGui::End();
    }

    _logic->on_render();

    return true;
}

u64 app::now()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

f64 app::get_delta() const
{
    return _delta;
}

void app::_window_size_callback(GLFWwindow* window, i32 width, i32 height)
{
    g_app->_desc->width = width;
    g_app->_desc->height = height;
    glViewport(0, 0, width, height);
    g_app->_logic->on_resize(width, height);
}

void app::_cursor_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    g_app->mouse_delta.x += xpos - g_app->mouse_pos.x;
    g_app->mouse_delta.y += g_app->mouse_pos.y - ypos;

    g_app->mouse_pos.x = xpos;
    g_app->mouse_pos.y = ypos;
}

void app::_mouse_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    if (action == GLFW_PRESS) {
        g_app->mouse_keys[button].down = true;
        g_app->mouse_keys[button].pressed = true;
    }
    else if (action == GLFW_RELEASE) {
        g_app->mouse_keys[button].down = false;
        g_app->mouse_keys[button].released = true;
    }
}

void app::_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (action == GLFW_PRESS) {
        g_app->keys[key].down = true;
        g_app->keys[key].pressed = true;
    }
    else if (action == GLFW_RELEASE) {
        g_app->keys[key].down = false;
        g_app->keys[key].released = true;
    }
}

void app::clear()
{
    // clear keyboard keys
    for (auto& key : keys) {
        key.pressed = false;
        key.released = false;
    }

    // clear mouse keys
    for (auto& key : mouse_keys) {
        key.pressed = false;
        key.released = false;
    }

    // clear mouse delta
    mouse_delta.x = 0;
    mouse_delta.y = 0;
}