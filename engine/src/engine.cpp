#include "engine.hpp"

#include "defines.hpp"
#include <renderer/resources/resources.hpp>
#include <Windows.h>
#include <iostream>
#include "glad/glad.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/resources/gl_errors.hpp"
#include "renderer/mesh.hpp"
#include "renderer/model.hpp"
#include "renderer/resources/buffer.hpp"
#include <utils.hpp>

std::unique_ptr<Engine> g_engine;

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

std::unique_ptr<Engine> Engine::create(std::unique_ptr<app_desc> desc)
{
    auto _app = std::make_unique<Engine>();
    _app->_desc = std::move(desc);
    return _app;
}

void Engine::add_logic(const std::string& dll_name)
{
    _logic = std::make_unique<game_logic>();
    _logic->load_game("testbedd.dll");
}

b8 Engine::init()
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

    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "Initialized debug layer" << std::endl;
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // create renderer
    m_renderer = Renderer::create();

    // create camera
    m_camera = Camera::create((f32)_desc->width / (f32)_desc->height, 45.0f, 0.1f, 100.0f);
    m_camera->set_position(glm::vec3(0.0f, 0.0f, 3.0f));

    // initialize camera matrices and uniform buffer
    _camera_matrices = std::make_shared<Matrices>();
    UniformBufferSpecification spec = {};
    spec.index = 0;
    spec.size = sizeof(Matrices);
    spec.usage = GL_DYNAMIC_DRAW;
    _matrices = std::make_shared<UniformBuffer>(spec);

    m_pbr = ShaderProgram::create("pbr.vert", "pbr.frag");
    m_model = Model::create("laptop");
    m_model->get_root()->m_transform = utils::create_transform(glm::vec3(0.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(10.0f));

    return _logic->on_init();

    return true;
}

void Engine::run()
{
    init();

    // opengl settings
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(_window))
    {
        glfwGetCurrentContext();

        update();

        {
            glClearColor(0.4f, 0.0f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        render();

        // clear just-pressed keys (do it before poll new events to avoid clearing keys that were pressed in the same frame)
        this->clear();

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
    }

    _logic->on_shutdown();
}

b8 Engine::update()
{
    // calculate delta time
    const auto now = this->now();
    _delta = float(now - _last_frame) / NS_PER_SECOND;
    _last_frame = now;

    // update camera
    Camera::Direction direction = Camera::Direction::NONE;
    if (keys[GLFW_KEY_W].down)
        m_camera->move(Camera::Direction::FORWARD, (f32)_delta);
    if (keys[GLFW_KEY_S].down)
        m_camera->move(Camera::Direction::BACKWARD, (f32)_delta);
    if (keys[GLFW_KEY_A].down)
        m_camera->move(Camera::Direction::LEFT, (f32)_delta);
    if (keys[GLFW_KEY_D].down)
        m_camera->move(Camera::Direction::RIGHT, (f32)_delta);
    if (keys[GLFW_KEY_SPACE].down)
        m_camera->move(Camera::Direction::UP, (f32)_delta);
    if (keys[GLFW_KEY_LEFT_SHIFT].down)
        m_camera->move(Camera::Direction::DOWN, (f32)_delta);

    m_camera->rotate((f32)mouse_delta.x, (f32)mouse_delta.y, (f32)_delta);

    // camera lock and unlock
    if (keys[GLFW_KEY_LEFT_CONTROL].pressed) {
        m_mouse_locked = !m_mouse_locked;
        if (m_mouse_locked) glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // update matrices
    _camera_matrices->projection = m_camera->get_projection_matrix();
    _camera_matrices->view = m_camera->get_view_matrix();
    _camera_matrices->eye_position = m_camera->get_position();
    _matrices->update(_camera_matrices.get(), sizeof(Matrices));

    // call game logic update
    _logic->on_update();

    if (keys[GLFW_KEY_ESCAPE].down) {
        glfwSetWindowShouldClose(_window, true);
    }

    return true;
}

b8 Engine::render()
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
    m_model->render(m_pbr);

    return true;
}

u64 Engine::now()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

f64 Engine::get_delta() const
{
    return _delta;
}

void Engine::_window_size_callback(GLFWwindow* window, i32 width, i32 height)
{
    g_engine->_desc->width = width;
    g_engine->_desc->height = height;
    glViewport(0, 0, width, height);
    g_engine->_logic->on_resize(width, height);
}

void Engine::_cursor_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    // initialize mouse position
    // this is done only once to avoid the mouse jumping to the center of the screen
    std::call_once(g_engine->m_mouse_init, [&]() {
        g_engine->mouse_pos.x = xpos;
        g_engine->mouse_pos.y = ypos;
        });

    g_engine->mouse_delta.x = xpos - g_engine->mouse_pos.x;
    g_engine->mouse_delta.y = g_engine->mouse_pos.y - ypos;

    g_engine->mouse_pos.x = xpos;
    g_engine->mouse_pos.y = ypos;
}

void Engine::_mouse_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    if (action == GLFW_PRESS) {
        g_engine->mouse_keys[button].down = true;
        g_engine->mouse_keys[button].pressed = true;
    }
    else if (action == GLFW_RELEASE) {
        g_engine->mouse_keys[button].down = false;
        g_engine->mouse_keys[button].released = true;
    }
}

void Engine::_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (action == GLFW_PRESS) {
        g_engine->keys[key].down = true;
        g_engine->keys[key].pressed = true;
    }
    else if (action == GLFW_RELEASE) {
        g_engine->keys[key].down = false;
        g_engine->keys[key].released = true;
    }
}

void Engine::clear()
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