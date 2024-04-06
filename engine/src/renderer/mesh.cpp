#include "mesh.hpp"

#include <format>
#include <string>
#include <iostream>
#include <defines.hpp>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/scene.h>

Mesh::Mesh(const aiMesh* mesh, const aiScene* scene, const std::string& model_path) {
    if (!mesh || !scene) {
        KERROR("Mesh or scene is nullptr for model at path: %s", model_path.c_str());
        return;
    }

    auto layout = VertexLayout::create();
    layout->push<f32>("position", 3);
    layout->push<f32>("normal", 3);
    layout->push<f32>("texcoord", 2);
    layout->push<f32>("tangent", 3);
    layout->push<f32>("bitangent", 3);

    // load vertices
    {
        std::vector<f32> vertices;
        vertices.reserve(mesh->mNumVertices * 16);
        for (u64 i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);

            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);

            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);

            vertices.push_back(mesh->mBitangents[i].x);
            vertices.push_back(mesh->mBitangents[i].y);
            vertices.push_back(mesh->mBitangents[i].z);
        }

        BufferSpecification vspec{};
        vspec.type = GL_ARRAY_BUFFER;
        vspec.count = (u32)vertices.size();
        vspec.data = vertices.data();
        vspec.element_size = sizeof(f32);
        vspec.usage = GL_STATIC_DRAW;
        m_vbuffer = GlBuffer::create(std::move(vspec));
    }

    // load indices
    {
        std::vector<u32> indices;
        indices.reserve(mesh->mNumFaces * 3);
        for (u64 i = 0; i < mesh->mNumFaces; i++) {
            auto face = mesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        BufferSpecification ispec{};
        ispec.type = GL_ELEMENT_ARRAY_BUFFER;
        ispec.count = (u32)indices.size();
        ispec.data = indices.data();
        ispec.element_size = sizeof(u32);
        ispec.usage = GL_STATIC_DRAW;
        m_ibuffer = GlBuffer::create(std::move(ispec));
    }

    VertexArraySpecification vao_spec{};
    vao_spec.layout = layout;
    vao_spec.index_buffer = m_ibuffer;
    vao_spec.vertex_buffer = m_vbuffer;
    m_vao = VertexArray::create(vao_spec);

    // load textures
    {
        const auto material = scene->mMaterials[mesh->mMaterialIndex];
        auto load_texture = [&](aiTextureType type, u32 slot) -> std::shared_ptr<Texture> {
            aiString texture_path;
            auto ret = material->GetTexture(type, 0, &texture_path);
            const auto path = std::filesystem::path(model_path) / texture_path.C_Str();
            KDEBUG("Loading texture: %s", path.string().c_str());

            TextureSpecification spec{};
            spec.slot = slot;
            spec.path = path.string();
            spec.internalFormat = type == aiTextureType_DIFFUSE ? GL_SRGB_ALPHA : GL_RGBA;
            spec.format = GL_RGB;
            return Texture::create(spec);
            };

        m_albedo = load_texture(aiTextureType_DIFFUSE, 0);
        m_normal = load_texture(aiTextureType_NORMALS, 1);
        m_mra = load_texture(aiTextureType_METALNESS, 2);
    }
}


void Mesh::draw(const std::shared_ptr<ShaderProgram> &shader, const glm::mat4 &model) const {
    shader->bind();
    shader->set_mat4("model", glm::value_ptr(model));

    if (m_albedo) m_albedo->bind();
    if (m_normal) m_normal->bind();
    if (m_mra) m_mra->bind();

    m_vao->bind();
    glDrawElements(GL_TRIANGLES, m_ibuffer->get_count(), GL_UNSIGNED_INT, nullptr);
    m_vao->unbind();
    shader->unbind();
}