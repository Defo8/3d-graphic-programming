#version 430

layout(std140, binding = 0) uniform Modifier {
 float strength;
 vec3  color; 
};

layout(location=0) out vec4 vFragColor;
in vec3 vColor;

void main() {
    vFragColor = vec4(vColor * color * strength, 1.0);
}