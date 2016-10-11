#version 450

layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec4 diffuse;
layout (location = 2) in vec3 emissive;
layout (location = 3) in vec3 normal;
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
//	fragColor = CalcColor();
}
