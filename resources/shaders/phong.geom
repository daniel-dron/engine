#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
} gs_in[];

out VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
} gs_out;

void main() {    
    for (int i = 0; i < 3; i++) {
        gs_out.normal = gs_in[i].normal;
        gs_out.uvs = gs_in[i].uvs;
        gs_out.fragPos = gs_in[i].fragPos;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive(); 
}
