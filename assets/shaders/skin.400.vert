#version 310 es
precision highp float;
in vec3 POSITION;
in vec3 NORMAL;
in vec2 vTexcoord;
in vec4 vColor;
in vec3 vBlendWeights;
in uvec4 vBlendIndices;
in uint materialId;
out vec2 texcoord;
out vec4 color;
out vec3 normal;
uniform mat4 matV;
uniform mat4 matP;
struct RenderCommand {
	mat4 matWorld;
	int meshId;
	uint boneStartIndex;
	int nBones;
	int padding;
};

struct Material {
	vec4 faceColor;
	vec3 specular;
	float power;
	vec3 emissive;
	int tmid;
};

layout (std140, binding = 2) uniform perInstanceUBO {
	RenderCommand renderCommands[10];
};
layout (std430, binding = 4) buffer materialSSBO {
	Material materials[];
};
layout (std430, binding = 5) buffer boneSSBO {
	mat4 bonesSSBO[];
};

void main() {
	RenderCommand cmd = renderCommands[gl_InstanceID];
	uint boneStartIndex = cmd.boneStartIndex;
	Material material = materials[materialId];

	mat4 comb =
		bonesSSBO[boneStartIndex + vBlendIndices.x] * vBlendWeights.x +
		bonesSSBO[boneStartIndex + vBlendIndices.y] * vBlendWeights.y +
		bonesSSBO[boneStartIndex + vBlendIndices.z] * vBlendWeights.z +
		bonesSSBO[boneStartIndex + vBlendIndices.w] * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);

	vec3 pos = POSITION.xyz;

	gl_Position = matP * matV * cmd.matWorld * comb * vec4(pos, 1.0);
	texcoord = vTexcoord;
	color = vColor * vec4(material.faceColor.xyz, 1.0) + vec4(material.emissive.xyz, 0.0);
	normal = mat3(cmd.matWorld * comb) * NORMAL;
}
