#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;

void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	float brightness = position.x < -0.8 || position.x > 0.8 ? 0.1 : 1.0;
	fragColor = vec4(brightness, brightness, brightness, 1.0);
//	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
