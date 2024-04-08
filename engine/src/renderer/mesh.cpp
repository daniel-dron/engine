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

#include <imgui/imgui.h>
#include <utils.hpp>
#include <engine.hpp>

Mesh::Mesh(const aiMesh* mesh, const aiScene* scene, const std::string& model_path)
	: m_name(mesh->mName.C_Str()) {
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
			KDEBUG("Loading texture: {}", path.string().c_str());

			TextureSpecification spec{};
			spec.slot = slot;
			spec.path = path.string();
			spec.internalFormat = type == aiTextureType_DIFFUSE || aiTextureType_EMISSIVE ? GL_SRGB_ALPHA : GL_RGBA;
			spec.format = GL_RGB;

			auto texture = g_engine->get_renderer()->get_texture(path.string());
			if (!texture) {
				texture = Texture::create(spec);
				g_engine->get_renderer()->add_texture(path.string(), texture);
			}

			return texture;
			};

		auto albedo = load_texture(aiTextureType_DIFFUSE, 0);
		auto normal = load_texture(aiTextureType_NORMALS, 1);
		auto mra = load_texture(aiTextureType_METALNESS, 2);
		auto emissive = load_texture(aiTextureType_EMISSIVE, 3);

		PbrMaterial pbr = {};
		pbr.metallic_factor = 1.0f;
		pbr.roughness_factor = 1.0f;
		pbr.ao_factor = 1.0f;
		pbr.albedo = albedo;
		pbr.normal = normal;
		pbr.mra = mra;
		pbr.emissive = emissive;
		pbr.shader = g_engine->get_renderer()->get_shader("pbr");
		m_pbr = std::make_shared<PbrMaterial>(pbr);
	}
}


void Mesh::render(const glm::mat4& model) const {
	m_pbr->bind();
	m_pbr->shader->set_mat4("model", glm::value_ptr(model));

	m_vao->bind();
	glDrawElements(GL_TRIANGLES, m_ibuffer->get_count(), GL_UNSIGNED_INT, nullptr);
}

std::string Mesh::get_name() const
{
	return m_name;
}

void Mesh::render_menu_debug() const
{
#if GRAPHICS_DEBUG
	ImGui::Text("Name: %s", m_name.c_str());
	
	m_pbr->render_menu_debug();
#endif
}
