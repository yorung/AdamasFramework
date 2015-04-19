#version 310 es

precision lowp float;
in vec2 texcoord;
in vec4 color;

layout (binding = 0) uniform sampler2D sampler;
layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = texture(sampler, texcoord) * color;
}
