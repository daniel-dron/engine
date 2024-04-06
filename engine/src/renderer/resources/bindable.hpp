#pragma once

#include <defines.hpp>

class KAPI Bindable {
public:
    u32 get_resource_id() const { return m_id; }

    virtual void bind() = 0;
    virtual void unbind() = 0;
protected:
    u32 m_id;
};