#version 310 es

precision highp float;
in vec2 vfPosition;
uniform vec4 b1[4];
uniform samplerCube s0;

layout (location = 0) out vec4 fragColor;

void main()
{
	mat4 invVP = mat4(b1[0], b1[1], b1[2], b1[3]);
	vec3 dir = normalize((invVP * vec4(vfPosition, 0.0, 1.0)).xyz);
	fragColor = texture(s0, dir);
}
