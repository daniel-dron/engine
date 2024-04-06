#include "defines.hpp"
#include "engine.hpp"

//
// Force .lib creation for the game/testbed to link to
//
__declspec(dllexport) int void_export() {
    return NULL;
}

__declspec(dllexport) void render_elements(u32 count);

__declspec(dllexport) std::string get_texture_path(const char* name);
