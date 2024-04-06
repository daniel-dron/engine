#include "buffer.hpp"

GlBuffer::GlBuffer(const BufferSpecification& spec)
    : m_type(spec.type), m_count(spec.count), m_element_size(spec.element_size) {
    glGenBuffers(1, &m_id);
    glBindBuffer(spec.type, m_id);
    glBufferData(spec.type, spec.element_size * spec.count, spec.data, spec.usage);
    glBindBuffer(spec.type, 0);
}

GlBuffer::~GlBuffer() {
    glDeleteBuffers(1, &m_id);
}

void GlBuffer::bind() {
    glBindBuffer(m_type, m_id);
}

void GlBuffer::unbind() {
    glBindBuffer(m_type, 0);
}

void GlBuffer::update(void* data, u32 count) {
    glBindBuffer(m_type, m_id);
    glBufferSubData(m_type, 0, m_element_size * count, data);
    glBindBuffer(m_type, 0);
}

UniformBuffer::UniformBuffer(const UniformBufferSpecification &spec)
    : m_index(spec.index), m_size(spec.size), m_usage(spec.usage) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);

    glBufferData(GL_UNIFORM_BUFFER, m_size, nullptr, m_usage);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_index, m_id);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() {
    glDeleteBuffers(1, &m_id);
}

void UniformBuffer::bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
}

void UniformBuffer::unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::update(void* data, u32 size) {
    this->bind();
    glBufferData(GL_UNIFORM_BUFFER, size, data, m_usage);
    this->unbind();
}