#version 310 es

precision mediump float;
in vec2 vfPosition;
in vec2 vfCoord;
uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
	vec2 coord = vfPosition * 0.5 + 0.5;
	float brightness = vfPosition.y < -0.8 || vfPosition.y > 0.8 ? 0.1 : 1.0;
	fragColor = vec4(texture(sampler, coord).xyz * brightness, 1.0);
}
