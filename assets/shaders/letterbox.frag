#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;
uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
	float brightness = position.y < -0.8 || position.y > 0.8 ? 0.1 : 1.0;
	fragColor = vec4(texture(sampler, texcoord).xyz * vec3(brightness, brightness, brightness), 1.0);
}
