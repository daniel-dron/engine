#pragma once

#include "resources/framebuffer.hpp"
#include "resources/shader_program.hpp"

class Model;
class IBL;

class RenderPass {
public:
	virtual void start();
	virtual void stop();
	virtual void render(const std::shared_ptr<Model>& model, const glm::mat4& transform);

	void set_shader(std::shared_ptr<ShaderProgram> shader);
	void set_framebuffer(std::shared_ptr<Framebuffer> framebuffer);

	void addDependencyBindable(std::shared_ptr<Bindable> bindable);
	void addOutputBindables(std::shared_ptr<Bindable> bindable);

	std::vector<std::shared_ptr<Bindable>> m_dependencies;
	std::vector<std::shared_ptr<Bindable>> m_outputs;
	std::shared_ptr<ShaderProgram> m_shader;
	std::shared_ptr<Framebuffer> m_framebuffer;
};

class GBuffer : public RenderPass {
public:
	GBuffer(FramebufferSpecification spec);

	std::shared_ptr<Texture> get_albedo();
	std::shared_ptr<Texture> get_normals();
	std::shared_ptr<Texture> get_mra();
	std::shared_ptr<Texture> get_emissive();
	std::shared_ptr<Texture> get_world();

	void bind_textures();
};

class LightingPass : public RenderPass {
public:
	LightingPass(FramebufferSpecification spec, std::shared_ptr<ShaderProgram> shader, std::vector<std::shared_ptr<Bindable>> gbuffer_textures, std::shared_ptr<IBL> ibl);

	void start() override;
private:
	std::shared_ptr<IBL> m_ibl;
};
