#pragma once

#include "resources/bindable.hpp"
#include <glad/glad.h>
#include <memory>

struct CubemapSpecification {
	u32 size = 512;
	GLenum internal_format = GL_RGB16F;
	GLenum data_format = GL_RGB;
	GLenum data_type = GL_FLOAT;
	u32 slot = 0;

	GLenum wrap_s = GL_CLAMP_TO_EDGE;
	GLenum wrap_t = GL_CLAMP_TO_EDGE;
	GLenum wrap_r = GL_CLAMP_TO_EDGE;
	GLenum min_filter = GL_LINEAR;
	GLenum mag_filter = GL_LINEAR;

	bool generate_mipmaps = false;
};

class Cubemap : public Bindable {
public:
	static std::shared_ptr<Cubemap> create(const CubemapSpecification& spec) {
		return std::make_shared<Cubemap>(spec);
	}

	Cubemap(const CubemapSpecification& spec);
	~Cubemap();

	void bind() override;
	void bind(u32 slot);
	void unbind() override;

	void generate_mipmap();
private:
	CubemapSpecification m_spec;
};