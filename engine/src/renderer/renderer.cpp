#include "renderer.hpp"

#include <iostream>
#include <format>
#include <imgui/imgui.h>
#include <utils.hpp>
#include <memory>

#include <engine.hpp>

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

	// initialize default pbr textures
	// TODO: this should be in a material system type class
	// but it's enough for current purposes
	init_default_pbr_material();
}

void Renderer::initialize() {
	// IBL
	auto path = ResourceState::get()->getTexturePath("kloofendal_overcast_puresky_8k.hdr");
	m_ibl = IBL::create(path);

	// create gbuffer
	init_gbuffer();

	// shadow map pass
	init_shadowmap_pass();

	// lighting pass
	init_lighting_pass();
}

void Renderer::init_default_pbr_material()
{
	auto albedo_path = ResourceState::get()->getTexturePath("default_albedo.png").string();
	auto mra_path = ResourceState::get()->getTexturePath("default_mra.png").string();
	auto normal_path = ResourceState::get()->getTexturePath("default_normal.png").string();
	auto emissive_path = ResourceState::get()->getTexturePath("default_emissive.png").string();

	{	// albedo
		TextureSpecification spec{};
		spec.slot = 0;
		spec.path = albedo_path;
		spec.internalFormat = GL_SRGB_ALPHA;
		spec.format = GL_RGB;
		auto texture = Texture::create(spec);
		add_texture(spec.path, texture);
	}

	{	// normal
		TextureSpecification spec{};
		spec.slot = 1;
		spec.path = normal_path;
		spec.internalFormat = GL_RGBA;
		spec.format = GL_RGB;
		auto texture = Texture::create(spec);
		add_texture(spec.path, texture);
	}

	{ // mra
		TextureSpecification spec{};
		spec.slot = 2;
		spec.path = mra_path;
		spec.internalFormat = GL_RGBA;
		spec.format = GL_RGB;
		auto texture = Texture::create(spec);
		add_texture(spec.path, texture);
	}

	{ // emissive
		TextureSpecification spec{};
		spec.slot = 3;
		spec.path = emissive_path;
		spec.internalFormat = GL_SRGB_ALPHA;
		spec.format = GL_RGB;
		auto texture = Texture::create(spec);
		add_texture(spec.path, texture);
	}

	PbrMaterial material = {};
	material.metallic_factor = 1.0f;
	material.roughness_factor = 1.0f;
	material.ao_factor = 1.0f;
	material.emissive_factor = 1.0f;
	material.albedo = get_texture(albedo_path);
	material.mra = get_texture(mra_path);
	material.normal = get_texture(normal_path);
	material.emissive = get_texture(emissive_path);
	material.shader = get_shader("pbr");
	add_pbr("default_pbr", std::make_shared<PbrMaterial>(material));
}

void Renderer::init_gbuffer() {
	//
	// create the 4 gbuffer textures
	//
	std::vector<std::shared_ptr<Texture>> color_attachements;
	TextureSpecification spec{};
	spec.width = 1920;
	spec.height = 1080;

	// 1. albedo (HDR)
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 0;
	color_attachements.push_back(Texture::create(spec));

	// 2. normals (HDR)
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 1;
	color_attachements.push_back(Texture::create(spec));

	// 3. mra (r-ambient occlusion, g-roughness, b-metallic)
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 2;
	color_attachements.push_back(Texture::create(spec));

	// 4. emissive (HDR)
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 3;
	color_attachements.push_back(Texture::create(spec));

	// 5. position (HDR for precision)
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 4;
	color_attachements.push_back(Texture::create(spec));

	//
	FramebufferSpecification frame_spec{};
	frame_spec.clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	frame_spec.color_attachements = color_attachements;
	frame_spec.depth_stencil = true;
	frame_spec.width = 1920;
	frame_spec.height = 1080;

	m_gbuffer = std::make_unique<GBuffer>(frame_spec);	// create gbuffer

	// set shader
	m_shaders["gbuffer"] = ShaderProgram::create("gbuffer.vert", "gbuffer.frag");
	m_gbuffer->m_shader = m_shaders["gbuffer"];

}

void Renderer::init_lighting_pass() {
	auto shader = ShaderProgram::create("deferred_lighting.vert", "deferred_lighting.frag");
	m_shaders["deferred_lighting"] = shader;

	TextureSpecification spec{};
	spec.width = 1920;
	spec.height = 1080;
	spec.internalFormat = GL_RGBA16F;
	spec.slot = 0;
	auto texture = Texture::create(spec);

	FramebufferSpecification frame_spec{};
	frame_spec.clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	frame_spec.color_attachements = { texture };
	frame_spec.depth_stencil = false;
	frame_spec.width = 1920;
	frame_spec.height = 1080;
	
	m_lighting_pass = std::make_unique<LightingPass>(frame_spec, shader, m_gbuffer->m_outputs, m_ibl, m_shadow_map_pass.get());
}

void Renderer::init_shadowmap_pass() {
	auto shader = ShaderProgram::create("shadow_map.vert", "shadow_map.frag");
	m_shaders["shadow_map"] = shader;

	FramebufferSpecification frame_spec{};
	frame_spec.clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	frame_spec.color_attachements = {};
	frame_spec.depth_stencil = false;

	m_shadow_map_pass = std::make_unique<ShadowMapPass>(frame_spec, shader);
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
	ImGui::Text("PBR default material");
	auto material = get_pbr("default_pbr");
	ImGui::Text("albedo: ");
	utils::imgui_render_hoverable_image(material->albedo, ImVec2(200.0f, 200.0f));
	ImGui::Text("normals: ");
	utils::imgui_render_hoverable_image(material->normal, ImVec2(200.0f, 200.0f));
	ImGui::Text("mra: ");
	utils::imgui_render_hoverable_image(material->mra, ImVec2(200.0f, 200.0f));
	ImGui::Text("emissive: ");
	utils::imgui_render_hoverable_image(material->emissive, ImVec2(200.0f, 200.0f));

	ImGui::Separator();
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

std::shared_ptr<PbrMaterial> Renderer::get_pbr(const std::string& name) const {
	auto material = m_pbr_materials.find(name);
	if (material == m_pbr_materials.end()) return nullptr;
	return material->second;
}

void Renderer::add_pbr(const std::string& name, std::shared_ptr<PbrMaterial> material) {
	m_pbr_materials[name] = material;
}
