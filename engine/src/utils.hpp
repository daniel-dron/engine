#pragma once

#include <glm/glm/glm.hpp>
#include <memory>

class Texture;
struct ImVec2;

namespace utils {
	glm::mat4 create_transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

	void imgui_render_hoverable_image(const std::shared_ptr<Texture>& texture, const ImVec2& render_size);
}