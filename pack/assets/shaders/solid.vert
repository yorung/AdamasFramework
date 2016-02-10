#version 310 es

precision highp float;
in vec3 POSITION;
in vec3 COLOR;
out vec3 color;

layout (std140, binding = 0) uniform perDrawCallUBO {
	mat4 matV;
	mat4 matP;
};

void main() {
	gl_Position = matP * matV * vec4(POSITION, 1);
	color = COLOR;
}
