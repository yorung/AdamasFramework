#version 310 es

out vec2 vfPosition;

void main() {
	vec2 vPosition = vec2((gl_VertexID & 2) != 0 ? 1.0 : -1.0, (gl_VertexID & 1) != 0 ? -1.0 : 1.0);
	gl_Position = vec4(vPosition.xy, -1, 1);
	vfPosition = vPosition;
}
