#pragma once 

#include "defines.hpp"
#include <string>
#include <functional>
#include <memory>
#include <string>

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

struct app {
public:
    std::unique_ptr<app_desc> desc;
    std::unique_ptr<game_logic> logic;
};

extern std::unique_ptr<app> g_app;