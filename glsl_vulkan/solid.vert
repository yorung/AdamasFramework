#version 450


layout(location = 0) out vec3 color;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(std140, binding = 0) uniform PerDraw
{
	mat4 matPV;
};

void main()
{
	gl_Position = matPV * vec4(inPosition, 1);
	color = inColor;
}
