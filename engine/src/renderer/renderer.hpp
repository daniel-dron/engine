#pragma once

#include <memory>

#include <defines.hpp>
#include "renderer/resources/resources.hpp"
#include "renderer/resources/shader_program.hpp"
#include "renderer/resources/vertex_array.hpp"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include "resources/framebuffer.hpp"

class Renderer {
public:
	static std::unique_ptr<Renderer> create() {
		return std::make_unique<Renderer>();
	}

	Renderer();

	void update_view(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& eye_pos);
	void render_screen_framebuffer(const std::shared_ptr<Framebuffer>& framebuffer);

	std::shared_ptr<ShaderProgram> get_shader(const std::string& name);

private:
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaders;

	struct ViewMatrices {
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 eye_position;
	};
	std::shared_ptr<ViewMatrices> m_view_matrices;
	std::shared_ptr<UniformBuffer> m_view_ub;

	// screen quad
	void init_screen_quad();
	std::unique_ptr<VertexArray> m_screen_vao;
	std::shared_ptr<GlBuffer> m_screen_ibo;
};
