#version 450

in vec3 POSITION;
in vec3 NORMAL;
in vec2 vTexcoord;
in vec4 vColor;
in vec3 vBlendWeights;
in uvec4 vBlendIndices;
in uint materialId;
out vec2 texcoord;
out vec4 diffuse;
out vec3 emissive;
out vec3 normal;
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
	Material materials[100];
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
	diffuse = vColor * vec4(materials[materialId].faceColor.xyz, 1.0);
	emissive = materials[materialId].emissive.xyz;
	normal = mat3(renderCommands[gl_InstanceIndex].matWorld * comb) * NORMAL;
}
