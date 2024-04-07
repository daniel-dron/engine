#pragma once

#include <defines.hpp>
#include <memory>
#include <string>
#include <glm/glm/glm.hpp>

#include "resources/texture.hpp"
#include "resources/buffer.hpp"
#include "resources/vertex_array.hpp"
#include "resources/shader_program.hpp"
#include "renderer.hpp"
#include "material.hpp"

// assimp forward declare
struct aiMesh;
struct aiScene;

class Mesh {
public:
    static std::shared_ptr<Mesh> create_from_assimp(const aiMesh *mesh, const aiScene *scene, const std::string &model_path) {
        return std::make_shared<Mesh>(mesh, scene, model_path);
    }

    Mesh(const aiMesh *mesh, const aiScene *scene, const std::string &model_path);

    void render(const glm::mat4 &model) const;
    std::string get_name() const;

    void render_menu_debug() const;
private:
    std::string m_name;

    std::shared_ptr<PbrMaterial> m_pbr;

    std::shared_ptr<GlBuffer> m_vbuffer;
    std::shared_ptr<GlBuffer> m_ibuffer;
    std::shared_ptr<VertexArray> m_vao;
};