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

uniform vec4 b0[4 * 3];		// matV, matP, matW;
uniform vec4 b1[3];			// material
uniform vec4 b2[100 * 4];	// bones

mat4 GetMatFromB0(int begin)
{
	return mat4(b0[begin + 0], b0[begin + 1], b0[begin + 2], b0[begin + 3]);
}

mat4 GetBone(uint index)
{
	uint i = index * 4u;
	return mat4(b2[i + 0u], b2[i + 1u], b2[i + 2u], b2[i + 3u]);
}

void main()
{
	mat4 matV = GetMatFromB0(0);
	mat4 matP = GetMatFromB0(4);
	mat4 matW = GetMatFromB0(8);

	vec4 faceColor = b1[0];
	emissive = b1[2].xyz;
	uvec4 boneIndices = uvec4(vBlendIndices);
	
	mat4 comb =
		GetBone(boneIndices.x) * vBlendWeights.x +
		GetBone(boneIndices.y) * vBlendWeights.y +
		GetBone(boneIndices.z) * vBlendWeights.z +
		GetBone(boneIndices.w) * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);

	vec3 pos = POSITION.xyz;

	gl_Position = matP * matV * matW * comb * vec4(pos, 1.0);
	texcoord = vTexcoord;
	diffuse = vColor * vec4(faceColor.xyz, 1.0);
	normal = mat3(matW * comb) * NORMAL;
}
