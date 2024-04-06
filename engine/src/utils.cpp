#include "utils.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <imgui/imgui.h>

#include "renderer/resources/texture.hpp"

glm::mat4 utils::create_transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	auto transform = glm::mat4(1.0f);
	transform = glm::translate(transform, position);

	transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = glm::scale(transform, scale);

	return transform;
}

void utils::imgui_render_hoverable_image(const std::shared_ptr<Texture>& texture, const ImVec2& render_size)
{
	ImGui::Image((ImTextureID)texture->get_resource_id(), render_size);
	if (ImGui::IsItemHovered()) {
		if (ImGui::BeginItemTooltip()) {
			ImGui::SetWindowSize({ 540, 600 });
			const glm::vec2 maxSize = glm::vec2{ 512.f, 512.f };
			auto scale = 1.f;
			if ((float)texture->get_width() > maxSize.x || (float)texture->get_height() > maxSize.y)
				scale = std::min(maxSize.x / texture->get_width(), maxSize.y / texture->get_height());
			ImGui::Image((void*)texture->get_resource_id(),
				ImVec2{
				texture->get_width() * scale, texture->get_height() * scale
				});
			ImGui::EndTooltip();
		}
	}
}
