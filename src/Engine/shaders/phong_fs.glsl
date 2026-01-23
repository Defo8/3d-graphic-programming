#version 460

layout(location=0) out vec4 vFragColor;

layout(std140, binding=0) uniform Modifiers {
    vec4  Kd;
    bool use_map_Kd;
};

in vec2 vertex_texcoords;
in vec3 vertex_normals_in_vs;
in vec3 vertex_coords_in_vs;

uniform sampler2D map_Kd;

const int MAX_POINT_LIGHTS=24;

struct PointLight {
    vec3 position_in_view_space;
    vec3 color;
    float intensity;
    float radius;
};

layout(std140, binding=2) uniform Lights {
    vec4 ambient;
    uint n_p_lights;
    PointLight p_light[MAX_POINT_LIGHTS];
};

void main() {
    vec3 normal = normalize(vertex_normals_in_vs);

    vec3 view_pos = vertex_coords_in_vs;

    vec3 diffuse_light = vec3(0.0);

    for (int i = 0; i < int(n_p_lights); i++) {
        PointLight light = p_light[i];

        vec3 light_dir = normalize(light.position_in_view_space - view_pos);

        float diff = max(dot(normal, light_dir), 0.0);

        diffuse_light += diff * light.color ;
    }

    vFragColor.a = Kd.a;
    vFragColor.rgb = Kd.rgb * (ambient.rgb * 0.0 + diffuse_light);
}