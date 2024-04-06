#include <stdio.h>
#include <entry.h>

#include <iostream>

extern "C" KAPI b8 on_init() {
    std::cout << "on_init()" << std::endl;
    return true;
}

extern "C" KAPI b8 on_update() {
    return true;
}

extern "C" KAPI b8 on_render() {
    return true;
}

extern "C" KAPI void on_resize(u32 width, u32 height) {
    std::cout << "on_resize()" << std::endl;
}

extern "C" KAPI void on_shutdown() {
    std::cout << "on_shutdown()" << std::endl;
}