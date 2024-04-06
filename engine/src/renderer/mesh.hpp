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

// assimp forward declare
struct aiMesh;
struct aiScene;

class Mesh {
public:
    static std::shared_ptr<Mesh> create_from_assimp(const aiMesh *mesh, const aiScene *scene, const std::string &model_path) {
        return std::make_shared<Mesh>(mesh, scene, model_path);
    }

    Mesh(const aiMesh *mesh, const aiScene *scene, const std::string &model_path);

    void draw(const std::shared_ptr<ShaderProgram> &shader, const glm::mat4 &model) const;

private:
    std::shared_ptr<Texture> m_albedo;
    std::shared_ptr<Texture> m_normal;
    std::shared_ptr<Texture> m_mra; // metallic, roughness, ao (gltf 2.0)

    std::shared_ptr<GlBuffer> m_vbuffer;
    std::shared_ptr<GlBuffer> m_ibuffer;
    std::shared_ptr<VertexArray> m_vao;
};