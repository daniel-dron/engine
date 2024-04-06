#include "renderer.hpp"

Renderer::Renderer() {
	// initialize camera matrices and uniform buffer
	m_view_matrices = std::make_shared<ViewMatrices>();
	UniformBufferSpecification spec = {};
	spec.index = 0;
	spec.size = sizeof(ViewMatrices);
	spec.usage = GL_DYNAMIC_DRAW;
	m_view_ub = std::make_shared<UniformBuffer>(spec);

	// initialize screen quad
	init_screen_quad();
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

void Renderer::init_screen_quad()
{
	std::shared_ptr<GlBuffer> vbuffer;
	{
		std::vector<f32> vertices = {
			// pos              // uvs
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};

		BufferSpecification vspec = {};
		vspec.type = GL_ARRAY_BUFFER;
		vspec.count = (u32)vertices.size();
		vspec.data = vertices.data();
		vspec.element_size = sizeof(f32);
		vspec.usage = GL_STATIC_DRAW;
		vbuffer = GlBuffer::create(std::move(vspec));
	}

	{
		std::vector<u32> indices = {
			0, 1, 2, 0, 2, 3
		};

		BufferSpecification ispec{};
		ispec.type = GL_ELEMENT_ARRAY_BUFFER;
		ispec.count = (u32)indices.size();
		ispec.data = indices.data();
		ispec.element_size = sizeof(u32);
		ispec.usage = GL_STATIC_DRAW;
		m_screen_ibo = GlBuffer::create(std::move(ispec));
	}

	auto layout = VertexLayout::create();
	layout->push<f32>("pos", 3).push<f32>("uvs", 2);

	VertexArraySpecification vao_spec{};
	vao_spec.layout = layout;
	vao_spec.index_buffer = m_screen_ibo;
	vao_spec.vertex_buffer = vbuffer;
	m_screen_vao = std::make_unique<VertexArray>(vao_spec);
}

void Renderer::render_screen_framebuffer(const std::shared_ptr<Framebuffer>& framebuffer)
{
	auto shader = get_shader("screen");

	shader->bind();
	m_screen_vao->bind();
	framebuffer->get_color_attachement(0)->bind();
	glDrawElements(GL_TRIANGLES, m_screen_ibo->get_count(), GL_UNSIGNED_INT, nullptr);
	m_screen_vao->unbind();
	shader->unbind();
}
