#version 310 es

out vec2 texcoord;
out vec2 position;

in int gl_VertexID;

void main() {
	vec2 vPosition = vec2((gl_VertexID & 1) != 0 ? 1 : -1, (gl_VertexID & 2) != 0 ? -1 : 1);

	gl_Position = vec4(vPosition.xy, 0, 1);
	texcoord.x = vPosition.x * 0.5 + 0.5;
	texcoord.y = vPosition.y * 0.5 + 0.5;
	position = vPosition;
}
