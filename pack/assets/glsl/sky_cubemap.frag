#version 310 es

precision highp float;
in vec2 vfPosition;

layout (location = 0) out vec4 fragColor;
layout (std140, binding = 1, row_major) uniform inst {
	mat4 invVP;
};
layout (binding = 0) uniform samplerCube tex;

void main() {
	vec3 dir = normalize((vec4(vfPosition, 0.0, 1.0) * invVP).xyz);
	fragColor = texture(tex, dir);
}
