#version 450

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec2 vTexcoord;
layout(location = 4) in vec3 vBlendWeights;
layout(location = 5) in uvec4 vBlendIndices;

layout (location = 0) out vec2 texcoord;
layout (location = 1) out vec4 diffuse;
layout (location = 2) out vec3 emissive;
layout (location = 3) out vec3 normal;
struct RenderCommand
{
	mat4 matWorld;
	int meshId;
	uint boneStartIndex;
	int nBones;
	int padding;
};

struct Material
{
	vec4 faceColor;
	vec3 specular;
	float power;
	vec3 emissive;
	int tmid;
};


layout (std140, set = 0, binding = 0) uniform perDrawCallUBO
{
	mat4 matV;
	mat4 matP;
	RenderCommand renderCommands[10];
};
layout (std140, set = 1, binding = 0) uniform materialUBO
{
	Material material;
};
layout (std140, set = 2, binding = 0) uniform boneSSBO
{
	mat4 bonesBuffer[100];
};

void main()
{
	uint boneStartIndex = renderCommands[gl_InstanceIndex].boneStartIndex;

	mat4 comb =
		bonesBuffer[boneStartIndex + vBlendIndices.x] * vBlendWeights.x +
		bonesBuffer[boneStartIndex + vBlendIndices.y] * vBlendWeights.y +
		bonesBuffer[boneStartIndex + vBlendIndices.z] * vBlendWeights.z +
		bonesBuffer[boneStartIndex + vBlendIndices.w] * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);

	vec3 pos = POSITION.xyz;

	gl_Position = matP * matV * renderCommands[gl_InstanceIndex].matWorld * comb * vec4(pos, 1.0);
	texcoord = vTexcoord;
	diffuse = vColor * vec4(material.faceColor.xyz, 1.0);
	emissive = material.emissive.xyz;
	normal = mat3(renderCommands[gl_InstanceIndex].matWorld * comb) * NORMAL;
}
