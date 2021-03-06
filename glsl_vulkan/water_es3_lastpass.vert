#version 450

layout (location = 0) out vec2 vfPosition;

void main()
{
	vec2 vPosition = vec2((gl_VertexIndex & 2) != 0 ? 1.0 : -1.0, (gl_VertexIndex & 1) != 0 ? -1.0 : 1.0);
	gl_Position = vec4(vPosition.xy, 0, 1);
	vfPosition = vPosition * vec2(1.0, -1.0);
}
