#version 450

layout (location = 0) out vec2 vfCoord;

void main()
{
	gl_Position = vec4((gl_VertexIndex & 2) != 0 ? 1.0 : -1.0, (gl_VertexIndex & 1) != 0 ? -1.0 : 1.0, 1.0, 1.0);
	vfCoord = gl_Position.xy * 0.5 + 0.5;
}
