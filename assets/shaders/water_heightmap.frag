#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
vec2 mousePos = fakeUBO[0].xy;
float elapsedTime = fakeUBO[0].z;

void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	float brightness = position.x < -0.8 || position.x > 0.8 ? 0.1 : 1.0;
//	brightness = sin(position.x + elapsedTime) + cos(position.y + elapsedTime);
	brightness = sin(log(length(position - mousePos)) * 10.0 - elapsedTime * 3.0) * 2.0 + 2.0 ;
	fragColor = vec4(brightness, brightness, brightness, 1.0);
}
