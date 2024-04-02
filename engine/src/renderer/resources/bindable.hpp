#pragma once

#include <defines.hpp>

class KAPI Bindable {
public:
    virtual void bind() = 0;
    virtual void unbind() = 0;
protected:
    u32 m_id;
};