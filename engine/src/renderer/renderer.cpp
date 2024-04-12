#include "renderer.hpp"

#include <iostream>
#include <format>

#include <imgui/imgui.h>

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

std::shared_ptr<Texture> Renderer::get_texture(const std::string& path) const
{
	std::shared_ptr<Texture> texture;

	if (m_textures.find(path) == m_textures.end()) {
		// create texture	
		return nullptr;
	}

	return m_textures.at(path);
}

void Renderer::add_texture(const std::string& path, std::shared_ptr<Texture> texture)
{
	m_textures[path] = texture;
}

void Renderer::init_screen_quad()
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
	m_screen_vbo = GlBuffer::create(std::move(vspec));

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

	auto layout = VertexLayout::create();
	layout->push<f32>("pos", 3).push<f32>("uvs", 2);

	VertexArraySpecification vao_spec{};
	vao_spec.layout = layout;
	vao_spec.index_buffer = m_screen_ibo;
	vao_spec.vertex_buffer = m_screen_vbo;
	m_screen_vao = std::make_unique<VertexArray>(vao_spec);
}

void Renderer::render_screen_framebuffer(const std::shared_ptr<Framebuffer>& framebuffer, u32 width, u32 height)
{
	auto shader = get_shader("screen");

	shader->bind();
	shader->set_bool("use_fxaa", use_fxaa);
	shader->set_float("luma_threshold", luma_threshold);
	shader->set_float("mul_reduce", 1.0f / mul_reduce);
	shader->set_float("min_reduce", 1.0f / min_reduce);
	shader->set_float("max_span", max_span);

	m_screen_vao->bind();
	framebuffer->get_color_attachement(0)->bind();
	framebuffer->get_color_attachement(1)->bind();
	shader->set_float("inverse_width", 1.0f / width);
	shader->set_float("inverse_height", 1.0f / height);
	glDrawElements(GL_TRIANGLES, m_screen_ibo->get_count(), GL_UNSIGNED_INT, nullptr);
	m_screen_vao->unbind();
	shader->unbind();
}

void Renderer::invalidate_shaders()
{
	for (auto& shader : m_shaders) {
		KDEBUG("Reloading shader {}...", shader.first.c_str());
		shader.second->invalidate();
	}
}

void Renderer::render_debug_menu()
{
	ImGui::Checkbox("Use FXAA", &use_fxaa);
	ImGui::BeginDisabled(!use_fxaa);
	ImGui::DragFloat("Threshold", &luma_threshold, 0.05f, 0.0f, 1.0f);
	ImGui::DragFloat("Max Span", &max_span, 1.0f, 1.0f, 16.0f);

	auto prev_mul_reduce = mul_reduce;
	ImGui::InputFloat("Mul Reduce", &mul_reduce, 0.1f, 0.1f);
	if (mul_reduce > prev_mul_reduce)
		mul_reduce = prev_mul_reduce * 2;
	else if (mul_reduce < prev_mul_reduce)
		mul_reduce = prev_mul_reduce / 2;
	mul_reduce = std::max(1.0f, mul_reduce);

	auto prev_min_reduce = min_reduce;
	ImGui::InputFloat("Min Reduce", &min_reduce, 0.1f, 0.1f);
	if (min_reduce > prev_min_reduce)
		min_reduce = prev_min_reduce * 2;
	else if (min_reduce < prev_min_reduce)
		min_reduce = prev_min_reduce / 2;
	min_reduce = std::max(1.0f, min_reduce);

	ImGui::EndDisabled();
}
