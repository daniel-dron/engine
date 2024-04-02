#pragma once

#include <glad/glad.h>

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                            const char *message, const void *userParam);

GLenum glCheckError_(const char *file, int line);

#if GRAPHICS_DEBUG
#define glCheckError() glCheckError_(__FILE__, __LINE__)
#else
#define glCheckError()
#endif

#define glcall(x)                                                                                                      \
    x;                                                                                                                 \
    glCheckError();
