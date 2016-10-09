#version 450

layout(location = 0) in vec2 POSITION;
layout(location = 1) in vec2 TEXCOORD;

out vec2 texcoord;

void main()
{
	gl_Position = vec4(POSITION.xy, 0, 1);
	texcoord = TEXCOORD;
}
