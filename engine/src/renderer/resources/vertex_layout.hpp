#pragma once

#include <glad/glad.h>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <string>

#include <defines.hpp>
#include <memory>

struct VertexElement {
    GLenum type;
    GLboolean normalized;
    u32 count;

    static u32 size_of(GLenum type) {
        switch (type) {
        case GL_FLOAT:
            return sizeof(f32);
        case GL_INT:
            return sizeof(i32);
        case GL_BYTE:
            return sizeof(b8);
        case GL_UNSIGNED_INT:
            return sizeof(u32);
        case GL_UNSIGNED_SHORT:
            return sizeof(u16);
        case GL_SHORT:
            return sizeof(i16);
        }
        return 0;
    }
};

class VertexLayout {
public:
    static std::shared_ptr<VertexLayout> create() {
        return std::make_shared<VertexLayout>();
    }

    VertexLayout() = default;

    template <class T>
    VertexLayout& push(const std::string& name, u32 count) {
        m_elements.push_back({type_dispatcher[typeid(T)], GL_FALSE, count});
        return *this;
    }

    const std::vector<VertexElement>& get_elements() const noexcept;

    u32 get_size() noexcept;
    
private:
    std::vector<VertexElement> m_elements;
    
    std::unordered_map<std::type_index, GLenum> type_dispatcher = {
        {typeid(f32), GL_FLOAT},
        {typeid(i32), GL_INT},
        {typeid(b8), GL_BYTE},
        {typeid(u32), GL_UNSIGNED_INT},
        {typeid(u16), GL_UNSIGNED_SHORT},
        {typeid(i16), GL_SHORT}
    };
};