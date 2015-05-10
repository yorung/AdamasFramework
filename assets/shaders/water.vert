#version 310 es

precision mediump float;
out vec2 position;

layout (location = 2) in vec2 vCoord;
in int gl_VertexID;

void main() {
	gl_Position = vec4(vCoord, 0, 1);
	position = vCoord;
}
