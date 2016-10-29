#version 450

in vec2 POSITION;
out vec2 texcoord;
out vec2 position;

void main()
{
	gl_Position = vec4(POSITION.xy, 1, 1);
	texcoord.x = POSITION.x * 0.5 + 0.5;
	texcoord.y = POSITION.y * 0.5 + 0.5;
	position = POSITION;
}
