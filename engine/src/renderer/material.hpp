#pragma once

#include "renderer/resources/texture.hpp"
#include "renderer/resources/shader_program.hpp"

struct PbrMaterial : public Bindable{
	float metallic_factor = 1.0f;
	float roughness_factor = 1.0f;
	float ao_factor = 1.0f;
	float emissive_factor = 1.0f;

	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normal;
	std::shared_ptr<Texture> mra;
	std::shared_ptr<Texture> emissive;
	std::shared_ptr<ShaderProgram> shader;

	// Inherited via Bindable
	void bind() override;
	void unbind() override;

	void render_menu_debug();
};