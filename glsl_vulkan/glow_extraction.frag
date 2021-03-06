#version 450

layout (location = 0) in vec2 vfPosition;
layout (location = 1) in vec2 vfCoord;
layout (set = 0, binding = 0) uniform sampler2D s0;
layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 src = texture(s0, vfCoord);
	float result = src.w;
	fragColor = vec4(result);
}
