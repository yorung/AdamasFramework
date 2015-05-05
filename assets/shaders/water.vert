#version 310 es

precision mediump float;
in vec3 vPosition;
in vec3 vNormal;
out vec2 position;

uniform vec4 fakeUBO[12];

in int gl_VertexID;

void main() {
	vec2 vPosition = vec2((gl_VertexID & 1) != 0 ? 1 : -1, (gl_VertexID & 2) != 0 ? -1 : 1);

	gl_Position = vec4(vPosition.xy, 0, 1);
	position = vPosition;
}
