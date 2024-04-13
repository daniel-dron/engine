#pragma once

#include "resources/framebuffer.hpp"
#include "resources/shader_program.hpp"

class GBuffer : public Framebuffer {
public:
	GBuffer(u32 width, u32 height);

	std::shared_ptr<Texture> get_albedo();
	std::shared_ptr<Texture> get_normals();
	std::shared_ptr<Texture> get_mra();
	std::shared_ptr<Texture> get_emissive();
	std::shared_ptr<Texture> get_world();

	void bind_textures();

private:
	static FramebufferSpecification create_spec(u32 width, u32 height);
};