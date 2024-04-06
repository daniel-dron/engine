#include "utils.hpp"
#include <glm/ext/matrix_transform.hpp>

glm::mat4 utils::create_transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	auto transform = glm::mat4(1.0f);
	transform = glm::translate(transform, position);

	transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = glm::scale(transform, scale);

	return transform;
}
