#include "defines.hpp"
#include "app.hpp"

//
// Force .lib creation for the game/testbed to link to
//
__declspec(dllexport) int void_export() {
    return NULL;
}

__declspec(dllexport) void render_elements(u32 count);
