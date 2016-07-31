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
out vec4 diffuse;
out vec3 emissive;
out vec3 normal;
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


layout (std140, binding = 0) uniform perDrawCallUBO {
	mat4 matV;
	mat4 matP;
	RenderCommand renderCommands[10];
};
layout (std140, binding = 1) uniform materialUBO {
	Material materials[100];
};
layout (std140, binding = 2) uniform boneSSBO {
	mat4 bonesBuffer[100];
};

void main() {
	RenderCommand cmd = renderCommands[gl_InstanceID];
	uint boneStartIndex = cmd.boneStartIndex;
	Material material = materials[materialId];

	mat4 comb =
		bonesBuffer[boneStartIndex + vBlendIndices.x] * vBlendWeights.x +
		bonesBuffer[boneStartIndex + vBlendIndices.y] * vBlendWeights.y +
		bonesBuffer[boneStartIndex + vBlendIndices.z] * vBlendWeights.z +
		bonesBuffer[boneStartIndex + vBlendIndices.w] * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);

	vec3 pos = POSITION.xyz;

	gl_Position = matP * matV * cmd.matWorld * comb * vec4(pos, 1.0);
	texcoord = vTexcoord;
	diffuse = vColor * vec4(material.faceColor.xyz, 1.0);
	emissive = material.emissive.xyz;
	normal = mat3(cmd.matWorld * comb) * NORMAL;
}
