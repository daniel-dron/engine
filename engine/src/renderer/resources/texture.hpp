#pragma once

#include <glad/glad.h>
#include <memory>
#include <string>

#include "defines.hpp"
#include "bindable.hpp"

struct TextureSpecification {
    GLenum target = GL_TEXTURE_2D;
    GLenum internalFormat = GL_RGBA;
    GLenum format = GL_RGBA;
    u32 width = 0;
    u32 height = 0;
    GLenum type = GL_UNSIGNED_BYTE;
    void* data = nullptr;
    std::string path = "";
    u32 slot = 0;
    GLenum wrapS = GL_REPEAT;
    GLenum wrapT = GL_REPEAT;
    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;
    bool generateMipmaps = true;
    bool hdr = false;
    bool flip_y = true;
};

class KAPI Texture : public Bindable {
public:
    static std::shared_ptr<Texture> create(const TextureSpecification& spec = TextureSpecification()) {
        return std::make_shared<Texture>(spec);
    }

    Texture(const TextureSpecification& spec);

    void bind() override;
    void unbind() override;
    void bind_to_framebuffer(u32 attachement_slot) const;

    u32 get_width() const;
    u32 get_height() const;
private:
    void loadFromFile();
    void loadHdrFromFile();
    void loadFromData();

    TextureSpecification m_spec;
    std::string m_path;
    u32 m_width;
    u32 m_height;
};