#include "ibl.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <engine.hpp>
#include "geometry.hpp"

IBL::IBL(const std::filesystem::path& hdr_path)
{
	_load_ibl_maps(hdr_path.string());
}

void IBL::bind(u32 irradiance_slot, u32 prefilter_slot, u32 brdf_slot)
{
	m_irradiance->bind(irradiance_slot);
	m_prefilter->bind(prefilter_slot);
	m_brdf->bind(brdf_slot);
}

void IBL::bind_env(u32 slot)
{
	m_env->bind(slot);
}

void IBL::_load_ibl_maps(const std::string& hdr_name)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	_initialize_ibl(hdr_name);
	_initialize_specular_ibl();
	_initialize_bdrf_texture();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void IBL::_initialize_ibl(const std::string& hdr_name)
{
	u32 width = 2560;
	u32 height = width;

	auto& renderer = g_engine->get_renderer();

	// capture framebuffer
	// will be used to render the hdri to various texture color attachements (irradiance, prefilter, brdf lut)
	FramebufferSpecification fspec{};
	fspec.clear_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	fspec.color_attachements.clear();
	fspec.height = height;
	fspec.width = width;
	fspec.depth_stencil = true;
	m_capture_framebuffer = Framebuffer::create(fspec);

	// hdri texture
	TextureSpecification spec{};
	spec.path = hdr_name;
	spec.hdr = true;
	spec.flip_y = false;
	m_hdr_texture = Texture::create(spec);

	// enviornment cube map
	CubemapSpecification env_spec{};
	env_spec.size = 2560;
	env_spec.internal_format = GL_RGB16F;
	env_spec.data_type = GL_FLOAT;
	env_spec.generate_mipmaps = true;
	m_env = Cubemap::create(env_spec);

	m_views = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	glViewport(0, 0, width, height);
	m_capture_framebuffer->bind();
	u32 attachements[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachements);
	for (u32 i = 0; i < 6; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_env->get_resource_id(), 0);
		auto cube = geometry::get_cube();
		cube->vao->bind();
		auto shader = renderer->get_shader("test");
		shader->bind();
		shader->set_mat4("projection", glm::value_ptr(m_projection));
		shader->set_mat4("view", glm::value_ptr(m_views[i]));
		m_hdr_texture->bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		cube->vao->unbind();
	}
	m_capture_framebuffer->unbind();


	// PBR: create an irradiance cubemap
	CubemapSpecification irr_spec{};
	irr_spec.size = 32;
	m_irradiance = Cubemap::create(irr_spec);

	// change capture framebuffer dimmensions
	m_capture_framebuffer->rescale(irr_spec.size, irr_spec.size);
	auto shader = renderer->get_shader("irradiance");
	shader->bind();
	m_env->bind(0);

	glViewport(0, 0, irr_spec.size, irr_spec.size);
	m_capture_framebuffer->bind();
	for (u32 i = 0; i < 6; i++) {
		shader->set_mat4("projection", glm::value_ptr(m_projection));
		shader->set_mat4("view", glm::value_ptr(m_views[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradiance->get_resource_id(), 0);

		auto cube = geometry::get_cube();
		cube->vao->bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		cube->vao->unbind();
	}
	m_capture_framebuffer->unbind();

	m_irradiance->bind();
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void IBL::_initialize_specular_ibl()
{
	auto& renderer = g_engine->get_renderer();

	auto shader = renderer->get_shader("prefilter");
	shader->bind();

	// create the prefilter map
	CubemapSpecification pre_spec{};
	pre_spec.size = 1024;
	pre_spec.generate_mipmaps = true;
	pre_spec.min_filter = GL_LINEAR_MIPMAP_LINEAR;
	m_prefilter = Cubemap::create(pre_spec);

	// bind the cubemap environment source texture
	m_env->bind(0);

	m_capture_framebuffer->bind();
	unsigned int max_mip_levels = 5;
	for (u32 mip = 0; mip < max_mip_levels; ++mip) {
		u32 mip_width = 1024 * std::pow(0.5f, mip);
		u32 mip_height = 1024 * std::pow(0.5f, mip);

		m_capture_framebuffer->rescale(mip_width, mip_height);
		glViewport(0, 0, mip_width, mip_height);

		f32 roughness = (f32)mip / (f32)(max_mip_levels - 1);
		shader->set_float("roughness", roughness);
		for (u32 i = 0; i < 6; i++) {
			shader->set_mat4("projection", glm::value_ptr(m_projection));
			shader->set_mat4("view", glm::value_ptr(m_views[i]));

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilter->get_resource_id(), mip);

			auto cube = geometry::get_cube();
			cube->vao->bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
			cube->vao->unbind();
		}
	}
	m_capture_framebuffer->unbind();
}

void IBL::_initialize_bdrf_texture()
{
	auto& renderer = g_engine->get_renderer();

	TextureSpecification brdf_spec{};
	brdf_spec.internalFormat = GL_RGB16F;
	brdf_spec.type = GL_FLOAT;
	brdf_spec.width = 512;
	brdf_spec.height = 512;
	m_brdf = Texture::create(brdf_spec);

	m_capture_framebuffer->bind();
	m_capture_framebuffer->rescale(512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdf->get_resource_id(), 0);
	glViewport(0, 0, 512, 512);

	auto shader = renderer->get_shader("brdf");
	shader->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderer->m_screen_vao->bind();
	glDrawElements(GL_TRIANGLES, renderer->m_screen_ibo->get_count(), GL_UNSIGNED_INT, nullptr);
	renderer->m_screen_vao->unbind();

	m_capture_framebuffer->unbind();
}
