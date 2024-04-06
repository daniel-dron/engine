#pragma once
#include <memory>
#include <vector>
#include "bindable.hpp"
#include "texture.hpp"
#include <glm/glm/glm.hpp>

struct FramebufferSpecification {
	std::vector<std::shared_ptr<Texture>> color_attachements;
	bool depth_stencil;
	u32 width;
	u32 height;
	glm::vec4 clear_color;
};

class KAPI Framebuffer : public Bindable {
public:
	static std::shared_ptr<Framebuffer> create(const FramebufferSpecification& spec) {
		return std::make_shared<Framebuffer>(spec);
	}

	Framebuffer(const FramebufferSpecification& spec);

	void bind() override;
	void unbind() override;
	void begin_pass();
	std::shared_ptr<Texture> get_color_attachement(u32 slot) const;
private:
	FramebufferSpecification m_spec;
	u32 m_color_attachement_id;
	u32 m_depth_stencil_renderbuffer;
};