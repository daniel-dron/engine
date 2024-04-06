#include "vertex_array.hpp"

#include <iostream>
#include <format>
#include "renderer/resources/gl_errors.hpp"
#include <defines.hpp>

VertexArray::VertexArray(const VertexArraySpecification& spec)
{
    glGenVertexArrays(1, &m_id);
    glBindVertexArray(m_id);

    spec.vertex_buffer->bind();
    spec.index_buffer->bind();

    u64 offset = 0;
    u32 idx = 0;
    u32 total_size = spec.layout->get_size();

    for (const auto& element : spec.layout->get_elements()) {
        GLCALL(glVertexAttribPointer(idx, element.count, element.type, element.normalized, total_size, (void*)offset));
        GLCALL(glEnableVertexAttribArray(idx));

        offset += element.count * VertexElement::size_of(element.type);
        idx++;
    }

    // unbind to not mess up gpu state
    glBindVertexArray(0);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void VertexArray::bind()
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind()
{
    glBindVertexArray(0);
}
