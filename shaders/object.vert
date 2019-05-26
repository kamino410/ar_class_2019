#version 400 core

in vec3 VertexPosition;
in vec2 VertexTex;

uniform mat4 mvp;

out vec3 norm;
out vec2 tex;

void main() {
    tex = VertexTex;

    gl_Position = mvp * vec4(VertexPosition, 1.0);
	//gl_Position = vec4(VertexPosition, 1.0);
}
