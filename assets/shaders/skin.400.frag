#version 430
in vec2 texcoord;
in vec4 color;
uniform sampler2D sampler;

layout (location = 0) out vec4 flagColor;

void main() {
//	flagColor = color;
	flagColor = texture(sampler, texcoord) * color;
	flagColor.w = 1.0;
//	flagColor = vec4(1, 1, 1, 1);
}
