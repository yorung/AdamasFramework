#version 310 es

precision highp float;
out vec2 vfPosition;

void main() {
	vec2 vPosition = vec2((gl_VertexID & 1) != 0 ? 1 : -1, (gl_VertexID & 2) != 0 ? -1 : 1);
	gl_Position = vec4(vPosition.xy, 0, 1);
	vfPosition = vPosition;
}
