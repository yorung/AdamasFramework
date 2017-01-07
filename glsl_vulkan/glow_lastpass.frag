#version 450

layout (location = 0) in vec2 vfPosition;
layout (location = 1) in vec2 vfCoord;

layout (set = 0, binding = 0) uniform sampler2D s0;
layout (set = 1, binding = 0) uniform sampler2D s1;
layout (set = 2, binding = 0) uniform sampler2D s2;
layout (set = 3, binding = 0) uniform sampler2D s3;
layout (set = 4, binding = 0) uniform sampler2D s4;
layout (set = 5, binding = 0) uniform sampler2D s5;
layout (set = 6, binding = 0) uniform sampler2D s6;

layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 org = texture(s6, vfCoord);
	vec4 g0 = texture(s0, vfCoord);
	vec4 g1 = texture(s1, vfCoord);
	vec4 g2 = texture(s2, vfCoord);
	vec4 g3 = texture(s3, vfCoord);
	vec4 g4 = texture(s4, vfCoord);
	vec4 g5 = texture(s5, vfCoord);
	fragColor = org + g0 + g1 + g2 + g3 + g4 + g5;
}
