#version 450

layout(location = 0) in vec2 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec4 COLOR;

layout (location = 0) out vec2 texcoord;
layout (location = 1) out vec4 color;

void main()
{
	gl_Position = vec4(POSITION.xy, 0, 1);
	texcoord = TEXCOORD;
	color = COLOR;
}
