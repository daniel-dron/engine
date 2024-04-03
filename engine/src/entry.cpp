#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <cassert>
#include <thread>



#include "renderer/resources/bindable.hpp"
#include "renderer/resources/resources.hpp"
#include "entry.h"
#include "renderer/resources/gl_errors.hpp"
#include "app.hpp"

void render_elements(u32 count) {
    GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

int main(int argc, char* argv[]) {
    ResourceState::get()->init(argv[0]);

    auto desc = std::make_unique<app_desc>();
    desc->pos_x = 100;
    desc->pos_y = 100;
    desc->width = 1920;
    desc->height = 1080;
    desc->window_name = "default window name";
    g_app = app::create(std::move(desc));

    g_app->add_logic("testbedd.dll");
    g_app->run();

    return 1;
}