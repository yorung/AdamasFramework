#version 310 es

precision highp float;
in vec2 vfPosition;
in vec2 vfCoord;

layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D glow0;
layout (binding = 1) uniform sampler2D glow1;
layout (binding = 2) uniform sampler2D glow2;
layout (binding = 3) uniform sampler2D glow3;
layout (binding = 4) uniform sampler2D glow4;
layout (binding = 5) uniform sampler2D glow5;
layout (binding = 6) uniform sampler2D org;

void main() {
	vec4 o = texture(org, vfCoord);
	vec4 g0 = texture(glow0, vfCoord);
	vec4 g1 = texture(glow1, vfCoord);
	vec4 g2 = texture(glow2, vfCoord);
	vec4 g3 = texture(glow3, vfCoord);
	vec4 g4 = texture(glow4, vfCoord);
	vec4 g5 = texture(glow5, vfCoord);
//	float result = src.w;
//	fragColor = vec4(result);

	fragColor = o + g0 + g1 + g2 + g3 + g4 + g5;
}
