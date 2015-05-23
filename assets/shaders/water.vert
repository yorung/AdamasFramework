#version 310 es

precision highp float;
out vec2 vfPosition;
uniform sampler2D waterHeightmap;

layout (location = 0) uniform vec4 fakeUBO[2];

vec2 heightMapSize = fakeUBO[1].zw;

void main() {
	vec2 vPosition = vec2((gl_VertexID & 1) != 0 ? 1 : -1, (gl_VertexID & 2) != 0 ? -1 : 1);
	gl_Position = vec4(vPosition.xy, 0, 1);
	vfPosition = vPosition;
}
