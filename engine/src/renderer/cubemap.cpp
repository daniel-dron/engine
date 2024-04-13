#include "cubemap.hpp"

Cubemap::Cubemap(const CubemapSpecification& spec)
{
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
	for (u32 i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, spec.internal_format, spec.size, spec.size, 0, spec.data_format, spec.data_type, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, spec.wrap_s);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, spec.wrap_t);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, spec.wrap_r);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, spec.min_filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, spec.mag_filter);

	if (spec.generate_mipmaps) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &m_id);
}

void Cubemap::bind()
{
	glActiveTexture(GL_TEXTURE0 + m_spec.slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::bind(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::unbind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::generate_mipmap()
{
	bind();
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
