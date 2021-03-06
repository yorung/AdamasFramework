#version 450

layout (location = 0) out vec2 position;
layout (location = 1) out vec2 texcoord;

void main()
{
	vec2 vPosition = vec2((gl_VertexIndex & 2) != 0 ? 1.0 : -1.0, (gl_VertexIndex & 1) != 0 ? -1.0 : 1.0);
	gl_Position = vec4(vPosition.xy, 1, 1);
	texcoord.x = vPosition.x * 0.5 + 0.5;
	texcoord.y = vPosition.y * 0.5 + 0.5;
	position = vPosition;
}
