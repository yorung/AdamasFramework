#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (binding = 0) uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	fragColor = vec4(texture(sampler, coord).xyz * 2.0, 1.0);
}
