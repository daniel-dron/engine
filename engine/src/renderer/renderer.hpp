#pragma once

#include <memory>

#include <defines.hpp>
#include "renderer/resources/resources.hpp"
#include "renderer/resources/shader_program.hpp"
#include "renderer/resources/vertex_array.hpp"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

struct DrawCall {
    std::shared_ptr<VertexArray> vao;
    std::shared_ptr<GlBuffer> ibo;
    glm::mat4 transform;
    std::shared_ptr<ShaderProgram> shader;
};

class Renderer {
public:
    static std::unique_ptr<Renderer> create() {
        return std::make_unique<Renderer>();
    }

    Renderer();

private:
    std::shared_ptr<ShaderProgram> m_pbr;
    std::vector<DrawCall> m_draw_calls;
};