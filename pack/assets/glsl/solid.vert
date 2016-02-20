#version 310 es

precision highp float;
in vec3 POSITION;
in vec3 COLOR;
out vec3 color;

layout (std140, binding = 0) uniform perDrawCallUBO {
	mat4 matPV;
};

void main() {
	gl_Position = matPV * vec4(POSITION, 1);
	color = COLOR;
}
