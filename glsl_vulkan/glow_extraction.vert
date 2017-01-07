#version 450

layout (location = 0) in vec2 POSITION;
layout (location = 0) out vec2 vfPosition;
layout (location = 1) out vec2 vfCoord;

void main()
{
	vfPosition = POSITION;
	gl_Position = vec4(vfPosition.xy, 0, 1);
	vfCoord = vfPosition * 0.5 + 0.5;
}
