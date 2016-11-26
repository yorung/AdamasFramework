precision highp float;
attribute vec3 POSITION;
attribute vec3 NORMAL;
attribute vec2 vTexcoord;
attribute vec4 vColor;
attribute vec3 vBlendWeights;
attribute vec4 vBlendIndices;
varying vec2 texcoord;
varying vec4 diffuse;
varying vec3 emissive;
varying vec3 normal;

uniform vec4 b0[4 * 3];		// matV, matP, matW;
uniform vec4 b1[3];			// material
uniform vec4 b2[50 * 4];	// bones

mat4 GetMatFromB0(int begin)
{
	return mat4(b0[begin + 0], b0[begin + 1], b0[begin + 2], b0[begin + 3]);
}

mat4 GetBone(int index)
{
	int i = index * 4;
	return mat4(b2[i + 0], b2[i + 1], b2[i + 2], b2[i + 3]);
}

void main()
{
	mat4 matV = GetMatFromB0(0);
	mat4 matP = GetMatFromB0(4);
	mat4 matW = GetMatFromB0(8);

	vec4 faceColor = b1[0];
	emissive = b1[2].xyz;
	ivec4 boneIndices = ivec4(vBlendIndices);
	
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
