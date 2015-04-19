#version 310 es
precision mediump float;
in vec2 texcoord;
in vec4 color;
layout (binding = 4) uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

void main() {
//	fragColor = color;
	fragColor = texture(sampler, texcoord) * color;
	fragColor.w = 1.0;
//	fragColor = vec4(1, 1, 1, 1);
}
