#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
vec2 mousePos = fakeUBO[0].xy;
float mouseDown = fakeUBO[0].z;
float elapsedTime = fakeUBO[0].w;

void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	float brightness = sin(log(length(position - mousePos)) * 10.0 - elapsedTime * 3.0) * 2.0 + 2.0 + mouseDown;
	fragColor = vec4(brightness, brightness, brightness, 1.0);
}
