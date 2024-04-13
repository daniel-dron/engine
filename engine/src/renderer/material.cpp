#include "material.hpp"

#include <utils.hpp>
#include <imgui/imgui.h>
#include <engine.hpp>
#include <iostream>

std::shared_ptr<PbrMaterial> PbrMaterial::from_assimp(const aiMaterial* ai_material, const std::string& model_path)
{
	auto& renderer = g_engine->get_renderer();

	// if the material was already loaded in the renderer codex, just reuse it for now
	auto material = renderer->get_pbr(ai_material->GetName().C_Str());
	if (material)
		return material;

	// create a copy of the default pbr material
	material = std::make_shared<PbrMaterial>(*renderer->get_pbr("default_pbr"));
	auto root_directory = std::filesystem::path(model_path);

	//
	// Get texture info from aiMaterial*, check if renderer codex already loaded it.
	// If not, create it and replace it on the new instance on material.
	//

	auto get_texture = [&](aiTextureType type, TextureSpecification spec) -> std::shared_ptr<Texture> {
		aiString path;
		auto ret = ai_material->GetTexture(type, 0, &path);
		if (ret != aiReturn_FAILURE) {
			auto texture_path = root_directory / path.C_Str();
			auto texture = renderer->get_texture(texture_path.string());

			// texture not on renderer codex
			if (!texture) {
				// create it and supply it to the codex
				spec.path = texture_path.string();
				texture = Texture::create(spec);
				KDEBUG("Loading texture: {}", spec.path.c_str());
				renderer->add_texture(texture_path.string(), texture);
			}

			return texture;
		}

		return nullptr;
		};


	{ // albedo
		TextureSpecification spec{};
		spec.slot = 0;
		spec.internalFormat = GL_SRGB_ALPHA;
		auto texture = get_texture(aiTextureType_DIFFUSE, spec);
		if (texture) material->albedo = texture;
	}

	{ // normals
		TextureSpecification spec{};
		spec.slot = 1;
		spec.internalFormat = GL_RGBA;
		auto texture = get_texture(aiTextureType_NORMALS, spec);
		if (texture) material->normal = texture;
	}

	{ // mra
		TextureSpecification spec{};
		spec.slot = 2;
		spec.internalFormat = GL_RGBA;
		auto texture = get_texture(aiTextureType_METALNESS, spec);
		if (texture) material->mra = texture;
	}

	{ // emissive
		TextureSpecification spec{};
		spec.slot = 3;
		spec.internalFormat = GL_SRGB_ALPHA;
		auto texture = get_texture(aiTextureType_EMISSIVE, spec);
		if (texture) material->emissive = texture;
	}

	return material;
}

void PbrMaterial::bind()
{
	shader->bind();
	shader->set_float("metallic_factor", metallic_factor);
	shader->set_float("roughness_factor", roughness_factor);
	shader->set_float("emissive_factor", emissive_factor);
	shader->set_float("ao_factor", ao_factor);

	if (albedo)
		albedo->bind();

	if (normal)
		normal->bind();

	if (mra)
		mra->bind();

	if (emissive)
		emissive->bind();
}

void PbrMaterial::unbind()
{
	shader->unbind();
}

void PbrMaterial::render_menu_debug()
{
#if GRAPHICS_DEBUG
	auto render_image_column = [&](const std::shared_ptr<Texture>& texture) {
		if (texture) utils::imgui_render_hoverable_image(texture, ImVec2(200.0f, 200.0f));
		else ImGui::Text("No Image!");
		};

	//ImGui::DragFloat("Metalness", &metallic_factor, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Roughness", &roughness_factor, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Ambient Occlusion", &ao_factor, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Emissive", &emissive_factor, 0.01f, 0.0f);

	// texture tables
	if (ImGui::BeginTable("Textures", 4)) {
		ImGui::TableSetupColumn("Albedo");
		ImGui::TableSetupColumn("Normal");
		ImGui::TableSetupColumn("MRA");
		ImGui::TableSetupColumn("Emissive");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		render_image_column(albedo);
		ImGui::TableNextColumn();
		render_image_column(normal);
		ImGui::TableNextColumn();
		render_image_column(mra);
		ImGui::TableNextColumn();
		render_image_column(emissive);

		ImGui::EndTable();
	}

#endif
}
