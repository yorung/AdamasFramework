#version 310 es

out vec2 texcoord;
out vec2 position;

in vec2 vPosition;

void main() {
	gl_Position = vec4(vPosition.xy, 0, 1);
	texcoord.x = vPosition.x * 0.5 + 0.5;
	texcoord.y = vPosition.y * 0.5 + 0.5;
	position = vPosition;
}
