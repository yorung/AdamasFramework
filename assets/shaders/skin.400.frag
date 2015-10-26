#version 310 es
precision mediump float;
in vec2 texcoord;
in vec4 color;
in vec3 normal;
layout (binding = 4) uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

vec4 CalcColor()
{
	vec4 d;
	d.xyz = clamp(dot(normalize(normal), normalize(vec3(0.0, 1.0, -1.0))), 0.0, 1.0) * color.xyz;
	d.w = 1.0f;
	return d;
}

void main() {
//	fragColor = color;
	fragColor = texture(sampler, texcoord) * CalcColor();
	fragColor.w = 1.0;
//	fragColor = vec4(1, 1, 1, 1);
}
