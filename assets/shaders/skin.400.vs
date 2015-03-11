#version 400
in vec3 POSITION;
in vec3 NORMAL;
in vec2 vTexcoord;
in vec4 vColor;
in vec3 vBlendWeights;
in vec4 vBlendIndices;
in uint drawId;
out vec2 texcoord;
out vec4 color;
uniform mat4 matW;
uniform mat4 matV;
uniform mat4 matP;
uniform mat4 bones[50];


void main() {
	mat4 matWV = matV * matW;

	mat4 comb =
		bones[int(vBlendIndices.x)] * vBlendWeights.x +
		bones[int(vBlendIndices.y)] * vBlendWeights.y +
		bones[int(vBlendIndices.z)] * vBlendWeights.z +
		bones[int(vBlendIndices.w)] * (1.0 - vBlendWeights.x - vBlendWeights.y - vBlendWeights.z);


	vec3 pos = POSITION.xyz;
	pos.x += drawId * 2.1;

	gl_Position = matP * matWV * comb * vec4(pos, 1);
	texcoord = vTexcoord;
	color = vColor;
}
