#pragma once 

#include "defines.hpp"
#include <string>
#include <functional>
#include <memory>
#include <string>
#include <array>
#include <glm/glm/glm.hpp>
#include <mutex>
#include "camera.hpp"

struct GLFWwindow;
class UniformBuffer;

struct app_desc {
    i32 pos_x;
    i32 pos_y;

    i32 width;
    i32 height;

    std::string window_name;
};

/**
 * @brief Structure representing the behavior of an application.
 */
struct game_logic {
    void load_game(const std::string& dll_name);

    /**
     * @brief Function called when the application is initialized.
     * @return True if initialization is successful, false otherwise.
     */
    std::function<b8(void)> on_init;

    /**
     * @brief Function called on each update frame of the application.
     * @return True if the update is successful, false otherwise.
     */
    std::function<b8(void)> on_update;

    /**
     * @brief Function called on each render frame of the application.
     * @return True if the render is successful, false otherwise.
     */
    std::function<b8(void)> on_render;

    /**
     * @brief Function called when the application window is resized.
     * @param width The new width of the window.
     * @param height The new height of the window.
     */
    std::function<void(u32 width, u32 height)> on_resize;

    /**
     * @brief Function called when the application is shutting down.
     */
    std::function<void(void)> on_shutdown;

    std::string get_current_dll_path() const;

    u32 current_dll_id;
    void* _dll;
};

struct key {
    // key is currently down
    bool down;
    // key was pressed this frame
    bool pressed;
    // key was released this frame
    bool released;
};

/**
 * @brief Represents an application.
 * 
 * This struct holds the description and logic of an application.
 */
struct app {
public:
    static std::unique_ptr<app> create(std::unique_ptr<app_desc> desc);

    void add_logic(const std::string &dll_name);
    b8 init();

    void run();

    b8 update();
    b8 render();

    u64 now();
    f64 get_delta() const;
    static const u64 NS_PER_SECOND = 1'000'000'000;

    std::array<key, 348> keys;
    std::array<key, 8> mouse_keys;
    vec2 mouse_pos;
    vec2 mouse_delta;
private:
    f64 _delta;
    u64 _last_frame; 

    // initialize camera matrices
    struct Matrices {
        glm::mat4 projection;
        glm::mat4 view;
    };
    std::shared_ptr<Matrices> _camera_matrices;
    std::shared_ptr<UniformBuffer> _matrices;
    std::shared_ptr<Camera> m_camera;

    std::once_flag m_mouse_init;
    bool m_mouse_locked = false;
    
    // clear keys
    void clear();

    static void _window_size_callback(GLFWwindow* window, i32 width, i32 height);
    static void _cursor_callback(GLFWwindow* window, f64 xpos, f64 ypos);
    static void _mouse_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
    static void _key_callback(GLFWwindow* window, i32 key, i32 scan_code, i32 action, i32 mods);

    GLFWwindow* _window; /**< The window of the application. */
    std::unique_ptr<app_desc> _desc; /**< The description of the application. */
    std::unique_ptr<game_logic> _logic; /**< The logic of the game. */
};

extern std::unique_ptr<app> g_app;