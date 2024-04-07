#include "material.hpp"

#include <utils.hpp>
#include <imgui/imgui.h>

void PbrMaterial::bind()
{
	shader->bind();
	shader->set_float("metallic_factor", metallic_factor);
	shader->set_float("roughness_factor", roughness_factor);
	shader->set_float("emissive_factor", emissive_factor);
	shader->set_float("ao_factor", ao_factor);

	albedo->bind();
	normal->bind();
	mra->bind();
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

	ImGui::DragFloat("Metalness", &metallic_factor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Roughness", &roughness_factor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Occlusion", &ao_factor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Emissive", &emissive_factor, 0.01f, 0.0f);

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
