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
	void render_screen_framebuffer(const std::shared_ptr<Framebuffer>& framebuffer, u32 width, u32 height);

	void invalidate_shaders();
	void render_debug_menu();
	std::shared_ptr<ShaderProgram> get_shader(const std::string& name);
	std::shared_ptr<Texture> get_texture(const std::string& path) const;
	void add_texture(const std::string& path, std::shared_ptr<Texture> texture);

	std::unique_ptr<VertexArray> m_screen_vao;
	std::shared_ptr<GlBuffer> m_screen_vbo;
	std::shared_ptr<GlBuffer> m_screen_ibo;
private:
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaders;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

	struct ViewMatrices {
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 eye_position;
	};
	std::shared_ptr<ViewMatrices> m_view_matrices;
	std::shared_ptr<UniformBuffer> m_view_ub;

    // FXAA
	float luma_threshold = 0.5f;
	float mul_reduce = 8.0f;
	float min_reduce = 128.0f;
	float max_span = 8.0f;
	bool use_fxaa = true;

	// screen quad
	void init_screen_quad();
};
