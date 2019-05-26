#version 400 core

in vec3 VertexPosition;
in vec2 VertexTex;

out vec3 pos;
out vec2 tex;

void main() {
    pos = VertexPosition;
    tex = VertexTex;
    gl_Position = vec4(pos, 1.0);
}
