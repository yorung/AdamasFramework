#version 310 es

precision highp float;
in vec3 POSITION;
in vec2 TEXCOORD;
in vec4 COLOR;
out vec2 texcoord;
out vec4 color;
layout (binding = 1) uniform matUbo {
	layout (row_major) mat4 matProj;
};

void main() {
	gl_Position = vec4(POSITION.xyz, 1) * matProj;
	texcoord = TEXCOORD;
	color = COLOR;
}
