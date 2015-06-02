#version 310 es

precision highp float;
in vec2 vfPosition;

layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D sourceMap;

void main() {
	vec2 coord = position * 0.5 + 0.5;
	vec4 src = texture(sourceMap, coord);
	float result = src.w;
	fragColor = vec4(result);
}
