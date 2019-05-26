#version 400 core

in vec3 norm;
in vec2 tex;

uniform sampler2D TexImg;

layout(location = 0) out vec4 fColor;

void main() {
    fColor = texture(TexImg, tex);
}
