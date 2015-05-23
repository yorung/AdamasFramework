#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;
uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
