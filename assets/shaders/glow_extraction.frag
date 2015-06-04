#version 310 es

precision highp float;
in vec2 vfPosition;
in vec2 vfCoord;

layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D sourceMap;

void main()
{
	vec4 src = texture(sourceMap, vfCoord);
//	float result = src.w;
//	fragColor = vec4(result);
	fragColor = src;
//	fragColor = vec4(vfCoord, 1.0, 1.0);
	fragColor = vec4(vfPosition, 1.0, 1.0);
}
