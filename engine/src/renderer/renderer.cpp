#include "renderer.hpp"

Renderer::Renderer() {
    // initialize camera matrices and uniform buffer
    m_view_matrices = std::make_shared<ViewMatrices>();
    UniformBufferSpecification spec = {};
    spec.index = 0;
    spec.size = sizeof(ViewMatrices);
    spec.usage = GL_DYNAMIC_DRAW;
    m_view_ub = std::make_shared<UniformBuffer>(spec);
}

void Renderer::update_view(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& eye_pos) {
    m_view_matrices->view = view;
    m_view_matrices->projection = projection;
    m_view_matrices->eye_position = eye_pos;
    m_view_ub->update(m_view_matrices.get(), sizeof(ViewMatrices));
}

std::shared_ptr<ShaderProgram> Renderer::get_shader(const std::string& name) {
	std::shared_ptr<ShaderProgram> shader;

	if (m_shaders.find(name) == m_shaders.end()) {
		// create shader
		m_shaders[name] = ShaderProgram::create(name + ".vert", name + ".frag");
	}

	return m_shaders[name];
}
