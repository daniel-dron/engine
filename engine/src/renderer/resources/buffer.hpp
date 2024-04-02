#pragma once

#include <glad/glad.h>
#include <defines.hpp>
#include <memory>

#include "bindable.hpp"

struct BufferSpecification {
    GLenum type;
    u32 element_size;
    u32 count;
    void* data;
    GLenum usage;
};

class KAPI GlBuffer : Bindable {
public:
    static std::shared_ptr<GlBuffer> create(const BufferSpecification &spec) {
        return std::make_shared<GlBuffer>(spec);
    }

    GlBuffer(const BufferSpecification &spec);
    ~GlBuffer();

    void bind() override;
    void unbind() override;
    void update(void* data, u32 count);

    u32 get_id() const { return m_id; }
    u32 get_count() const { return m_count; }
    u32 get_element_size() const { return m_element_size; }
private:
    GLenum m_type;
    u32 m_count;
    u32 m_element_size;
};