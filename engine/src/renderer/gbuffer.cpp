#include "gbuffer.hpp"

#include <engine.hpp>
#include "model.hpp"
#include <imgui/imgui.h>
#include <utils.hpp>

GBuffer::GBuffer(FramebufferSpecification spec) {
	m_framebuffer = Framebuffer::create(spec);
	m_outputs.insert(m_outputs.end(), spec.color_attachements.begin(), spec.color_attachements.end());
}

std::shared_ptr<Texture> GBuffer::get_albedo() {
	return std::dynamic_pointer_cast<Texture>(m_outputs.at(0));
}

std::shared_ptr<Texture> GBuffer::get_normals() {
	return std::dynamic_pointer_cast<Texture>(m_outputs.at(1));
}

std::shared_ptr<Texture> GBuffer::get_mra() {
	return std::dynamic_pointer_cast<Texture>(m_outputs.at(2));
}

std::shared_ptr<Texture> GBuffer::get_emissive() {
	return std::dynamic_pointer_cast<Texture>(m_outputs.at(3));
}

std::shared_ptr<Texture> GBuffer::get_world() {
	return std::dynamic_pointer_cast<Texture>(m_outputs.at(4));

}

void GBuffer::bind_textures() {
	get_albedo()->bind();
	get_normals()->bind();
	get_mra()->bind();
	get_emissive()->bind();
	get_world()->bind();
}

void RenderPass::start() {
	m_shader->bind();
	m_framebuffer->begin_pass();

	for (const auto& bindable : m_dependencies) {
		bindable->bind();
	}

	for (const auto& bindable : m_outputs) {
		bindable->bind();
	}
}

void RenderPass::stop() {
	m_shader->unbind();
	m_framebuffer->unbind();
}

void RenderPass::render(const std::shared_ptr<Model>& model, const glm::mat4& transform) {
	model->render(m_shader, transform);
}

void RenderPass::set_shader(std::shared_ptr<ShaderProgram> shader) {
	m_shader = shader;
}

void RenderPass::set_framebuffer(std::shared_ptr<Framebuffer> framebuffer) {
	m_framebuffer = framebuffer;
}

void RenderPass::addDependencyBindable(std::shared_ptr<Bindable> bindable) {
	m_dependencies.push_back(std::move(bindable));
}

void RenderPass::addOutputBindables(std::shared_ptr<Bindable> bindable) {
	m_outputs.push_back(std::move(bindable));
}


LightingPass::LightingPass(
	FramebufferSpecification spec,
	std::shared_ptr<ShaderProgram> shader,
	std::vector<std::shared_ptr<Bindable>> gbuffer_textures,
	std::shared_ptr<IBL> ibl, ShadowMapPass* shadow_pass) {

	m_framebuffer = Framebuffer::create(spec);
	m_shader = shader;
	m_ibl = ibl;
	m_shadow_pass = shadow_pass;

	m_dependencies.insert(m_dependencies.begin(), gbuffer_textures.begin(), gbuffer_textures.end());
}

void LightingPass::start() {
	RenderPass::start();

	m_ibl->bind(5, 6, 7);

	auto light_space = m_shadow_pass->get_light_space();
	auto shadow_map = m_shadow_pass->get_depth_texture();

	m_shader->set_mat4("light_space_matrix", glm::value_ptr(light_space));
	shadow_map->bind(8);
}

ShadowMapPass::ShadowMapPass(FramebufferSpecification spec, std::shared_ptr<ShaderProgram> shader) {
	m_shader = shader;

	// create depth texture
	TextureSpecification tspec{};
	tspec.internalFormat = GL_DEPTH_COMPONENT;
	tspec.format = GL_DEPTH_COMPONENT;
	tspec.type = GL_FLOAT;
	tspec.width = 12024;
	tspec.height = 12024;
	tspec.wrapS = GL_CLAMP_TO_BORDER;
	tspec.wrapT = GL_CLAMP_TO_BORDER;
	tspec.minFilter = GL_NEAREST;
	tspec.magFilter = GL_NEAREST;
	tspec.attachement_target = GL_DEPTH_ATTACHMENT;
	
	m_shadow_texture = std::make_shared<Texture>(tspec);

	// set border
	m_shadow_texture->bind();
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	m_shadow_texture->unbind();
	
	spec.color_attachements = { m_shadow_texture };
	m_framebuffer = Framebuffer::create(spec);

}

void ShadowMapPass::set_light_position(const glm::vec3& pos) {
	light_position = pos;
}

void ShadowMapPass::start() {
	RenderPass::start();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT);

	glViewport(0, 0, 12024, 12024);
	auto light_space = get_light_space();

	m_shader->bind();
	m_shader->set_mat4("light_space_matrix", glm::value_ptr(light_space));
}

void ShadowMapPass::stop() {
	RenderPass::stop();
	glCullFace(GL_BACK);
}

std::shared_ptr<Texture> ShadowMapPass::get_depth_texture() {
	return m_shadow_texture;
}

glm::mat4 ShadowMapPass::get_light_space() {
	glm::mat4 proj = glm::ortho(-bounds, bounds, -bounds, bounds, near_plane, far_plane);
	glm::mat4 view = glm::lookAt(light_position * m_distance, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 light_space = proj * view;
	return light_space;
}

void ShadowMapPass::render_debug_menu() {
	ImGui::Begin("ShadowMapPass");
	ImGui::DragFloat("bounds", &bounds, 0.01f);
	ImGui::DragFloat("distance", &m_distance, 0.01f);
	ImGui::DragFloat3("position", glm::value_ptr(light_position), 0.01f);
	ImGui::DragFloat("near", &near_plane, 0.01f);
	ImGui::DragFloat("far", &far_plane, 0.01f);

	utils::imgui_render_hoverable_image(m_shadow_texture, ImVec2(400.0f, 400.0f));
	ImGui::End();
}
