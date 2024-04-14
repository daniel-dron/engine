#include "gbuffer.hpp"

#include <engine.hpp>
#include "model.hpp"

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
	std::shared_ptr<IBL> ibl) {

	m_framebuffer = Framebuffer::create(spec);
	m_shader = shader;
	m_ibl = ibl;

	m_dependencies.insert(m_dependencies.begin(), gbuffer_textures.begin(), gbuffer_textures.end());
}

void LightingPass::start() {
	RenderPass::start();

	m_ibl->bind(5, 6, 7);
}
