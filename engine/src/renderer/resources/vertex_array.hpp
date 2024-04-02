#pragma once

#include <glad/glad.h>
#include <memory>

#include "bindable.hpp"
#include "vertex_layout.hpp"
#include "buffer.hpp"

struct VertexArraySpecification {
    std::shared_ptr<VertexLayout> layout;
    std::shared_ptr<GlBuffer> vertex_buffer;
    std::shared_ptr<GlBuffer> index_buffer;
};

class KAPI VertexArray : Bindable {
public:
    static std::shared_ptr<VertexArray> create(const VertexArraySpecification &spec) {
        return std::make_shared<VertexArray>(spec);
    }

    VertexArray(const VertexArraySpecification &spec);
    ~VertexArray();

    void bind() override;
    void unbind() override;
};