#pragma once

#include <memory>
#include <glm/glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>

#include "resources/framebuffer.hpp"
#include "cubemap.hpp"

class IBL {
public:
	static std::shared_ptr<IBL> create(const std::filesystem::path &hdr_path) {
		return std::make_shared<IBL>(hdr_path);
	}

	IBL(const std::filesystem::path &hdr_path);

	void bind(u32 irradiance_slot, u32 prefilter_slot, u32 brdf_slot);
	void bind_env(u32 slot);

private:
	// original hdri image
	std::shared_ptr<Texture> m_hdr_texture = nullptr;

	// cubemap used to render the hdri skybox
	std::shared_ptr<Cubemap> m_env = nullptr;

	std::shared_ptr<Cubemap> m_irradiance = nullptr;
	std::shared_ptr<Cubemap> m_prefilter = nullptr;
	std::shared_ptr<Texture> m_brdf = nullptr;
	
	// capture framebuffer to render to various textures
	std::shared_ptr<Framebuffer> m_capture_framebuffer = nullptr;

	glm::mat4 m_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.0f, 10.0f);
	std::vector<glm::mat4> m_views;

	void _load_ibl_maps(const std::string& hdr_name);
	void _initialize_ibl(const std::string& hdr_name);
	void _initialize_specular_ibl();
	void _initialize_bdrf_texture();
};
