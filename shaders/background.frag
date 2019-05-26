#version 400 core

in vec3 pos;
in vec2 tex;

uniform sampler2DRect TexImg;
uniform float TexHeight;
uniform float TexWidth;

layout(location = 0) out vec4 fColor;

void main() {
    fColor = texture(TexImg, vec2(TexWidth, TexHeight)*tex);
}
