#version 450

in vec2 texcoord;

layout (set = 0, binding = 0) uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(tex, texcoord);
//	fragColor = vec4(texture(tex, texcoord).xyz, 1.0);
//	fragColor = vec4(0.5, 0.5, 1.0, 1.0) + texture(tex, texcoord);
}
