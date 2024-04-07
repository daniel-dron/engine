#include "shader_program.hpp"

#include <glad/glad.h>
#include <fstream>
#include <iostream>

#include "resources.hpp"
#include "gl_errors.hpp"

ShaderProgram::ShaderProgram(const std::string& vertex_name, const std::string& fragment_name) {
    const auto vertex_path = ResourceState::get()->getShaderPath(vertex_name);
    const auto fragment_path = ResourceState::get()->getShaderPath(fragment_name);

    m_vertex_path = vertex_path.string();
    m_frag_path = fragment_path.string();

    m_id = compile_shader(vertex_path, fragment_path);
}

void ShaderProgram::checkCompileErrors(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if (!success) {
            GLCALL(glGetShaderInfoLog(shader, 1024, nullptr, infoLog));
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        GLCALL(glGetProgramiv(shader, GL_LINK_STATUS, &success));
        if (!success) {
            GLCALL(glGetProgramInfoLog(shader, 1024, nullptr, infoLog));
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

u32 ShaderProgram::compile_shader(const std::filesystem::path& vertex_path, const std::filesystem::path& frag_path)
{
    std::string vertex_code;
    std::string fragment_code;

    std::ifstream v_shader_file;
    std::ifstream f_shader_file;

    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        v_shader_file.open(vertex_path);
        f_shader_file.open(frag_path);
        std::stringstream v_shader_stream, f_shader_stream;

        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();

        v_shader_file.close();
        f_shader_file.close();

        vertex_code = v_shader_stream.str();
        fragment_code = f_shader_stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();

    // compile
    u32 vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    u32 id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return id;
}

void ShaderProgram::bind() { GLCALL(glUseProgram(m_id)); }

void ShaderProgram::unbind() { GLCALL(glUseProgram(0)); }

void ShaderProgram::invalidate()
{
    glDeleteProgram(m_id);
    m_id = compile_shader(m_vertex_path, m_frag_path);
}

void ShaderProgram::set_bool(const std::string &name, bool value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniform1i(loc, (int)value));
}

void ShaderProgram::set_int(const std::string &name, int value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniform1i(loc, value));
}

void ShaderProgram::set_float(const std::string &name, float value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniform1f(loc, value));
}

void ShaderProgram::set_vec3(const std::string &name, float *value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniform3f(loc, value[0], value[1], value[2]));
}

void ShaderProgram::set_vec2(const std::string &name, float *value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniform2f(loc, value[0], value[1]));
}

void ShaderProgram::set_mat4(const std::string &name, const float *value) const {
    const auto loc = GLCALL(glGetUniformLocation(m_id, name.c_str()));
    GLCALL(glUniformMatrix4fv(loc, 1, GL_FALSE, value));
}