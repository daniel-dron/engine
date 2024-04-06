#pragma once

#include <glm/glm/glm.hpp>

namespace utils {
	glm::mat4 create_transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
}