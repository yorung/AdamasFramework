#version 310 es
precision mediump float;
in vec2 texcoord;
in vec4 diffuse;
in vec3 normal;
in vec3 emissive;
layout (binding = 4) uniform sampler2D sampler;

layout (location = 0) out vec4 fragColor;

vec4 CalcColor()
{
	vec4 d;
	d.xyz = clamp(dot(normalize(normal), normalize(vec3(0.0, 1.0, -1.0))), 0.0, 1.0) * diffuse.xyz + emissive;
	d.w = 1.0f;
	return d;
}

void main() {
	fragColor = texture(sampler, texcoord) * CalcColor();
	fragColor.w = 1.0;
}
