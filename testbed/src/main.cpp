#include <stdio.h>
#include <entry.h>

#include <iostream>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/shader_program.hpp>
#include <renderer/resources/vertex_array.hpp>
#include <renderer/resources/vertex_layout.hpp>

extern "C" KAPI void init_game_window(app_desc * desc) {
    desc->pos_x = 100;
    desc->pos_y = 100;

    desc->width = 1920;
    desc->height = 1080;

    desc->window_name = "dev: architecture";
}

std::shared_ptr<ShaderProgram> shader;
std::shared_ptr<GlBuffer> vbuffer;
std::shared_ptr<GlBuffer> ibuffer;
std::shared_ptr<VertexArray> vao;

extern "C" KAPI b8 on_init() {
    std::cout << "on_init()" << std::endl;
    
    // try to compile a shader
    shader = ShaderProgram::create("test.vert", "test.frag");
    shader->bind();

    // createa a simple triangle vertices
    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    //indices
    u32 indices[] = {
        0, 1, 2
    };

    // create a vertex buffer
    BufferSpecification buffer_spec = {};
    buffer_spec.data = vertices;
    buffer_spec.type = GL_ARRAY_BUFFER;
    buffer_spec.element_size = sizeof(f32);
    buffer_spec.count = sizeof(vertices) / sizeof(f32);
    buffer_spec.usage = GL_STATIC_DRAW;
    vbuffer = GlBuffer::create(buffer_spec);

    // create an index buffer
    buffer_spec.data = indices;
    buffer_spec.type = GL_ELEMENT_ARRAY_BUFFER;
    buffer_spec.element_size = sizeof(u32);
    buffer_spec.count = sizeof(indices) / sizeof(u32);
    buffer_spec.usage = GL_STATIC_DRAW;
    ibuffer = GlBuffer::create(buffer_spec);

    // create a vertex layout
    auto layout = VertexLayout::create();
    layout->push<f32>("position", 3);

    // create a vertex array
    VertexArraySpecification va_spec = {};
    va_spec.index_buffer = ibuffer;
    va_spec.vertex_buffer = vbuffer;
    va_spec.layout = std::move(layout);
    vao = VertexArray::create(va_spec);

    vao->unbind();
    vbuffer->unbind();
    ibuffer->unbind();

    return true;
}

extern "C" KAPI b8 on_update() {
    return true;
}

extern "C" KAPI b8 on_render() {
    shader->bind();
    vao->bind();
    
    render_elements(ibuffer->get_count());

    return true;
}

extern "C" KAPI void on_resize(u32 width, u32 height) {
    std::cout << "on_resize()" << std::endl;
}

extern "C" KAPI void on_shutdown() {
    std::cout << "on_shutdown()" << std::endl;
}