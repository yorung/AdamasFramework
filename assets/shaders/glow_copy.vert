#version 310 es

out vec2 vfPosition;
out vec2 vfCoord;


void main() {
	vfPosition = vec2((gl_VertexID & 1) != 0 ? 1.0 : -1.0, (gl_VertexID & 2) != 0 ? -1.0 : 1.0);
	gl_Position = vec4(vfPosition.xy, 0, 1);
	vfCoord = vfPosition * 0.5 + 0.5;
}
