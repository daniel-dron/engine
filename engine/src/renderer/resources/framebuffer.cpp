#include "framebuffer.hpp"
#include <iostream>
#include <format>
#include <cassert>

Framebuffer::Framebuffer(const FramebufferSpecification& spec)
	: m_spec(spec), m_color_attachement_id(0)
{
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);


	// create depth renderbuffer
	if (m_spec.depth_stencil) {
		glGenRenderbuffers(1, &m_depth_stencil_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_spec.width, m_spec.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_stencil_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	for (const auto& texture : m_spec.color_attachements) {
		texture->bind_to_framebuffer(m_color_attachement_id++);
	}

	std::vector<u32> attachments;
	for (int i = 0; i < m_color_attachement_id; i++) {
		if (m_spec.color_attachements[i]->get_spec().attachement_target == GL_DEPTH_ATTACHMENT)
			continue;

		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	if (attachments.size() == 0.0f) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	} else {
		glDrawBuffers(attachments.size(), attachments.data());
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		KERROR("Framebuffer is not complete!");
		assert(false);
	}

	// always unbind after creation
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::begin_pass()
{
	this->bind();

	glClearColor(m_spec.clear_color.r, m_spec.clear_color.g, m_spec.clear_color.b, m_spec.clear_color.a);
	glClear(GL_COLOR_BUFFER_BIT | (m_spec.depth_stencil ? (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) : 0x0));
}

std::shared_ptr<Texture> Framebuffer::get_color_attachement(u32 slot) const
{
	return m_spec.color_attachements[slot];
}

void Framebuffer::rescale(u32 width, u32 height)
{
	bind();
	if (m_spec.depth_stencil) {
		glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	}
}
