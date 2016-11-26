#version 310 es
precision highp float;
in vec3 POSITION;
in vec3 NORMAL;
in vec2 vTexcoord;
in vec4 vColor;
in vec3 vBlendWeights;
in vec4 vBlendIndices;
in float materialId;
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

layout (std140, binding = 0) uniform perDrawCallUBO
{
	mat4 matV;
	mat4 matP;
	RenderCommand renderCommands[10];
};

uniform vec4 b1[3];

layout (std140, binding = 2) uniform boneUBO
{
	mat4 bonesBuffer[100];
};

void main()
{
	vec4 faceColor = b1[0];
	emissive = b1[2].xyz;

	RenderCommand cmd = renderCommands[gl_InstanceID];
	uint boneStartIndex = cmd.boneStartIndex;
	uvec4 boneIndices = boneStartIndex + uvec4(vBlendIndices);
	
	mat4 comb =
		bonesBuffer[boneIndices.x] * vBlendWeights.x +
		bonesBuffer[boneIndices.y] * vBlendWeights.y +
		bonesBuffer[boneIndices.z] * vBlendWeights.z +
		bonesBuffer[boneIndices.w] * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);

	vec3 pos = POSITION.xyz;

	gl_Position = matP * matV * cmd.matWorld * comb * vec4(pos, 1.0);
	texcoord = vTexcoord;
	diffuse = vColor * vec4(faceColor.xyz, 1.0);
	normal = mat3(cmd.matWorld * comb) * NORMAL;
}
