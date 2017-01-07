#version 450

layout(location = 0) in vec3 POSITION;
layout(location = 2) in vec2 TEXCOORD;
layout(location = 1) in vec4 COLOR;
layout (location = 0) out vec2 texcoord;
layout (location = 1) out vec4 color;

layout (set = 1, binding = 0) uniform matUbo
{
	layout (row_major) mat4 matProj;
};

void main()
{
	gl_Position = vec4(POSITION.xyz, 1) * matProj;
	texcoord = TEXCOORD;
	color = COLOR;
}
