#include "buffer.hpp"

GlBuffer::GlBuffer(const BufferSpecification& spec)
    : m_type(spec.type), m_count(spec.count), m_element_size(spec.element_size) {
    glGenBuffers(1, &m_id);
    glBindBuffer(spec.type, m_id);
    glBufferData(spec.type, spec.element_size * spec.count, spec.data, spec.usage);
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