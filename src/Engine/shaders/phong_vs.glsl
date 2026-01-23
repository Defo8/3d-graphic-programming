#version 460

layout(location=0) in vec4 a_vertex_position;
layout(location=1) in vec2 a_vertex_texcoords;
layout(location=5) in vec3 a_vertex_normals;

layout(std140, binding=1) uniform Transformations {
    mat4 PVM;
    mat4 VM;
    mat3 N;  
};

out vec2 vertex_texcoords;
out vec3 vertex_normals_in_vs;
out vec3 vertex_coords_in_vs;

void main() {
    vertex_texcoords = a_vertex_texcoords;
    
    vertex_coords_in_vs = (VM * a_vertex_position).xyz;
    
    vertex_normals_in_vs = normalize(N * a_vertex_normals);
    
    gl_Position =  PVM * a_vertex_position;
}