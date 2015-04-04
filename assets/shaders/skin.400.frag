#version 310 es
precision mediump float;
in vec2 texcoord;
in vec4 color;
layout (binding = 4) uniform sampler2D sampler;

layout (location = 0) out vec4 flagColor;

void main() {
//	flagColor = color;
	flagColor = texture(sampler, texcoord) * color;
	flagColor.w = 1.0;
//	flagColor = vec4(1, 1, 1, 1);
}
