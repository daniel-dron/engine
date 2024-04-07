#pragma once
#include <string>

#include "bindable.hpp"
#include <filesystem>
#include <memory>

class KAPI ShaderProgram : Bindable {
  public:
    static std::shared_ptr<ShaderProgram> create(const std::string &vertex_name, const std::string &fragment_name) {
        return std::make_shared<ShaderProgram>(vertex_name, fragment_name);
    }

    ShaderProgram(const std::string &vertex_name, const std::string &fragment_name);
    ShaderProgram(const std::string &vertex_name, const std::string &fragment_name, const std::string &geometry_name);

    // delete copy and move constructors
    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;
    ShaderProgram(ShaderProgram &&) = delete;
    ShaderProgram &operator=(ShaderProgram &&) = delete;

    void bind() override;
    void unbind() override;
    void invalidate();

    void set_bool(const std::string &name, bool value) const;
    void set_int(const std::string &name, int value) const;
    void set_float(const std::string &name, float value) const;
    void set_mat4(const std::string &name, const float *value) const;
    void set_vec3(const std::string &name, float *value) const;
    void set_vec2(const std::string &name, float *value) const;

  private:
    static void checkCompileErrors(unsigned int shader, const std::string &type);
    static u32 compile_shader(const std::filesystem::path& vertex_path, const std::filesystem::path& frag_path);

    std::string m_vertex_path;
    std::string m_frag_path;
};
