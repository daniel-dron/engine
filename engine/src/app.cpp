#include "app.hpp"

#include "defines.hpp"
#include <renderer/resources/resources.hpp>
#include <Windows.h>
#include <iostream>

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