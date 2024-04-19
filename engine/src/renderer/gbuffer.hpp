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

class ShadowMapPass : public RenderPass {
public:
	ShadowMapPass(FramebufferSpecification spec, std::shared_ptr<ShaderProgram> shader);

	void set_light_position(const glm::vec3& pos);
	void start() override;
	void stop() override;

	std::shared_ptr<Texture> get_depth_texture();
	glm::mat4 get_light_space();

	void render_debug_menu();
private:
	float near_plane = 0.010f;
	float far_plane = 25.0f;
	float bounds = 13.0f;
	float m_distance = 6.0f;
	
	glm::vec3 light_position = glm::vec3(1.0f);
	std::shared_ptr<Texture> m_shadow_texture;
};


class LightingPass : public RenderPass {
public:
	LightingPass(FramebufferSpecification spec, std::shared_ptr<ShaderProgram> shader, std::vector<std::shared_ptr<Bindable>> gbuffer_textures, std::shared_ptr<IBL> ibl, ShadowMapPass* shadow_pass);

	void start() override;
private:
	std::shared_ptr<IBL> m_ibl;
	ShadowMapPass* m_shadow_pass;
};
