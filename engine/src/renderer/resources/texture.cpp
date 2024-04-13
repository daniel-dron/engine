#include "texture.hpp"

#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "resources.hpp"
#include <defines.hpp>

#include "gl_errors.hpp"

Texture::Texture(const TextureSpecification& spec) : m_spec(spec) {
	// from file
	if (m_spec.data == nullptr && m_spec.path != "") {
		if (m_spec.hdr) loadHdrFromFile();
		else loadFromFile();
	}
	// from data
	else
		loadFromData();
}

void Texture::bind() {
	glActiveTexture(GL_TEXTURE0 + m_spec.slot);
	GLCALL(glBindTexture(m_spec.target, m_id));
}

void Texture::bind(u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	GLCALL(glBindTexture(m_spec.target, m_id));
}

void Texture::unbind() {
	glBindTexture(m_spec.target, 0);
}

void Texture::bind_to_framebuffer(u32 attachement_slot) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachement_slot, m_spec.target, m_id, 0);
}

void Texture::loadHdrFromFile() {
	auto path = m_spec.path;

	// if the file does not exist, load the missing texture
	if (!std::filesystem::exists(m_spec.path)) {
		path = ResourceState::get()->getTexturePath("missing.png").string();
		assert(std::filesystem::exists(path) && "Missing texture not found");
		KERROR("Texture not found: {}", m_spec.path);
	}

	i32 width, height, channels;
	if (m_spec.flip_y)
		stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	if (!data) {
		KERROR("Failed to load texture: {}", path);
		stbi_image_free(data);

		// try to load the missing texture
		path = ResourceState::get()->getTexturePath("missing.png").string();
		assert(std::filesystem::exists(path) && "Missing texture not found");
		data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	}

	m_width = width;
	m_height = height;

	glGenTextures(1, &m_id);
	glBindTexture(m_spec.target, m_id);

	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_spec.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_spec.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(m_spec.target, 0, GL_RGB16F, width, height, 0, channels > 3 ? GL_RGBA : GL_RGB, GL_FLOAT, data);

	glBindTexture(m_spec.target, 0);
	stbi_image_free(data);
}

void Texture::loadFromFile() {
	auto path = m_spec.path;

	// if the file does not exist, load the missing texture
	if (!std::filesystem::exists(m_spec.path)) {
		path = ResourceState::get()->getTexturePath("missing.png").string();
		assert(std::filesystem::exists(path) && "Missing texture not found");
		KERROR("Texture not found: {}", m_spec.path);
	}

	i32 width, height, channels;
	if (m_spec.flip_y)
		stbi_set_flip_vertically_on_load(true);
	u8* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (!data) {
		KERROR("Failed to load texture: {}", path);
		stbi_image_free(data);

		// try to load the missing texture
		path = ResourceState::get()->getTexturePath("missing.png").string();
		assert(std::filesystem::exists(path) && "Missing texture not found");
		data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	}

	m_width = width;
	m_height = height;

	glGenTextures(1, &m_id);
	glBindTexture(m_spec.target, m_id);

	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_S, m_spec.wrapS);
	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_T, m_spec.wrapT);
	glTexParameteri(m_spec.target, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
	glTexParameteri(m_spec.target, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

	glTexImage2D(m_spec.target, 0, m_spec.internalFormat, width, height, 0, channels > 3 ? GL_RGBA : GL_RGB, m_spec.type, data);
	if (m_spec.generateMipmaps)
		glGenerateMipmap(m_spec.target);

	glBindTexture(m_spec.target, 0);
	stbi_image_free(data);
}

void Texture::loadFromData() {
	glGenTextures(1, &m_id);
	glBindTexture(m_spec.target, m_id);

	m_width = m_spec.width;
	m_height = m_spec.height;

	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_S, m_spec.wrapS);
	glTexParameteri(m_spec.target, GL_TEXTURE_WRAP_T, m_spec.wrapT);
	glTexParameteri(m_spec.target, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
	glTexParameteri(m_spec.target, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

	glTexImage2D(m_spec.target, 0, m_spec.internalFormat, m_spec.width, m_spec.height, 0, m_spec.format, m_spec.type, m_spec.data);

	if (m_spec.generateMipmaps)
		glGenerateMipmap(m_spec.target);

	glBindTexture(m_spec.target, 0);
}

u32 Texture::get_width() const
{
	return m_width;
}

u32 Texture::get_height() const
{
	return m_height;
}
