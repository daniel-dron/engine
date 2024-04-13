#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <cassert>
#include <thread>

#include "renderer/resources/bindable.hpp"
#include "renderer/resources/resources.hpp"
#include "entry.h"
#include "renderer/resources/gl_errors.hpp"
#include "engine.hpp"

void render_elements(u32 count) {
    GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

std::string get_texture_path(const char* name)
{
    return ResourceState::get()->getTexturePath(name).string();
}

int main(int argc, char* argv[]) {
    ResourceState::get()->init(argv[0]);

    auto desc = std::make_unique<app_desc>();
    desc->pos_x = 100;
    desc->pos_y = 100;
    desc->width = 1920;
    desc->height = 1080;
    desc->fullscreen = false;
    desc->window_name = "branch: dev-pbr";
    g_engine = Engine::create(std::move(desc));

    g_engine->add_logic("testbedd.dll");
    g_engine->run();

    return 1;
}