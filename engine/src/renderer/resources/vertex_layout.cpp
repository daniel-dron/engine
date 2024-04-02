#include "vertex_layout.hpp"

const std::vector<VertexElement>& VertexLayout::get_elements() const noexcept
{
    return m_elements;
}

u32 VertexLayout::get_size() noexcept
{
    u32 size = 0;
    for (const auto& element : m_elements) {
        size += element.count * VertexElement::size_of(element.type);
    }

    return size;
}
