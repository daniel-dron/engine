#include "gbuffer.hpp"
#include <engine.hpp>

GBuffer::GBuffer(u32 width, u32 height) : Framebuffer(create_spec(width, height)) {
}

std::shared_ptr<Texture> GBuffer::get_albedo() {
	return m_spec.color_attachements.at(0);
}

std::shared_ptr<Texture> GBuffer::get_normals() {
	return m_spec.color_attachements.at(1);
}

std::shared_ptr<Texture> GBuffer::get_mra() {
	return m_spec.color_attachements.at(2);
}

std::shared_ptr<Texture> GBuffer::get_emissive() {
	return m_spec.color_attachements.at(3);
}

std::shared_ptr<Texture> GBuffer::get_world() {
	return m_spec.color_attachements.at(4);
}

FramebufferSpecification GBuffer::create_spec(u32 width, u32 height) {
	//
	// create the 4 gbuffer textures
	//
	
	std::vector<std::shared_ptr<Texture>> color_attachements;
	TextureSpecification spec{};
	spec.width = width;
	spec.height = height;

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
	frame_spec.width = width;
	frame_spec.height = height;

	return frame_spec;
}

void GBuffer::bind_textures() {
	get_albedo()->bind();
	get_normals()->bind();
	get_mra()->bind();
	get_emissive()->bind();
	get_world()->bind();
}
