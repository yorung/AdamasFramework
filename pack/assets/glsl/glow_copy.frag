#version 310 es

precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;

layout (binding = 0) uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
	vec4 src = texture(sampler, vfCoord);
	fragColor = src;
}
