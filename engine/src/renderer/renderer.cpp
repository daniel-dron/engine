#include "renderer.hpp"

Renderer::Renderer() {
    m_pbr = ShaderProgram::create("pbr.vert", "pbr.frag");
}

