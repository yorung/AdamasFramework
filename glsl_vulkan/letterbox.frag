#version 450

#define texture2D texture

layout (location = 0) in vec2 vfPosition;
layout (location = 1) in vec2 vfCoord;

layout (binding = 0) uniform sampler2D s0;

layout (location = 0) out vec4 fragColor;

void main()
{
	vec2 coord = vfPosition * 0.5 + 0.5;
	float brightness = vfPosition.y < -0.8 || vfPosition.y > 0.8 ? 0.1 : 1.0;
	fragColor = vec4(texture2D(s0, coord).xyz * brightness, 1.0);
}
