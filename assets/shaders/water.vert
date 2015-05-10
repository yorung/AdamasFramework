#version 310 es

precision mediump float;
out vec2 position;


in int gl_VertexID;
void main() {
	vec2 vPosition = vec2((gl_VertexID & 1) != 0 ? 1 : -1, (gl_VertexID & 2) != 0 ? -1 : 1);

	gl_Position = vec4(vPosition.xy, 0, 1);
	position = vPosition;
}
/*

in vec2 vCoord;
void main() {
	vec2 vPosition = vCoord;

	gl_Position = vec4(vPosition.xy, 0, 1);
	position = vPosition;
}
*/