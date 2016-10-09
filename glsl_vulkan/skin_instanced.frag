#version 450

in vec2 texcoord;
in vec4 diffuse;
in vec3 normal;
in vec3 emissive;
layout (set = 3, binding = 0) uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;

vec4 CalcColor()
{
	vec4 d;
	d.xyz = clamp(dot(normalize(normal), normalize(vec3(0.0, 1.0, -1.0))), 0.0, 1.0) * diffuse.xyz + emissive;
	d.w = 1.0f;
	return d;
}

void main()
{
	fragColor = texture(tex, texcoord) * CalcColor();
	fragColor.w = 1.0;
}
