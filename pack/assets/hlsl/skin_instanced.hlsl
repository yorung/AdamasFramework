struct VS_INPUT {
	float3 POSITION : POSITION;
	float3 NORMAL : NORMAL;
	float2 vTexcoord : vTexcoord;
	float4 vColor : vColor;
	float3 vBlendWeights : vBlendWeights;
	uint4 vBlendIndices : vBlendIndices;
	uint materialId : materialId;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float2 texcoord : texcoord;
	float4 diffuse : diffuse;
	float3 emissive : emmisive;
	float3 normal : normal;
};

struct RenderCommand {
	matrix matWorld;
	int meshId;
	uint boneStartIndex;
	int nBones;
	int padding;
};

struct Material {
	float4 faceColor;
	float3 specular;
	float power;
	float3 emissive;
	int tmid;
};

cbuffer perDrawCallUBO : register(b0)
{
	matrix matV;
	matrix matP;
	RenderCommand renderCommands[10];
};

cbuffer materialUBO : register(b1)
{
	Material material;
};

cbuffer boneUBO : register(b2)
{
	matrix bonesBuffer[100];
};

#define RSDEF "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), CBV(b0), CBV(b1), CBV(b2), DescriptorTable(SRV(t3)), StaticSampler(s3)"

[RootSignature(RSDEF)]
VS_OUTPUT VSMain(VS_INPUT _In, uint instanceId : SV_InstanceID)
{
	RenderCommand cmd = renderCommands[instanceId];
	uint boneStartIndex = cmd.boneStartIndex;

	matrix comb =
		mul(bonesBuffer[boneStartIndex + _In.vBlendIndices.x], _In.vBlendWeights.x) +
		mul(bonesBuffer[boneStartIndex + _In.vBlendIndices.y], _In.vBlendWeights.y) +
		mul(bonesBuffer[boneStartIndex + _In.vBlendIndices.z], _In.vBlendWeights.z) +
		mul(bonesBuffer[boneStartIndex + _In.vBlendIndices.w], 1.0 - _In.vBlendWeights.x - _In.vBlendWeights.y - _In.vBlendWeights.z);

	float3 pos = _In.POSITION.xyz;

	VS_OUTPUT Out = (VS_OUTPUT)0;

	matrix matWC = mul(cmd.matWorld, comb);	
	Out.position = mul(mul(mul(matP, matV), matWC), float4(pos, 1.0));
	Out.texcoord = _In.vTexcoord;
	Out.diffuse = _In.vColor * float4(material.faceColor.xyz, 1.0);
	Out.emissive = material.emissive.xyz;
	Out.normal = mul((float3x3)matWC, _In.NORMAL);

	return Out;
}

float4 CalcColor(VS_OUTPUT _In)
{
	float4 d;
	d.xyz = _In.emissive.xyz + saturate(dot(normalize(_In.normal), normalize(float3(0,1,-1)))) * _In.diffuse.xyz;
	d.w = 1.0f;
	return d;
}

SamplerState gSampler : register(s3);
Texture2D gTexture : register(t3);

[RootSignature(RSDEF)]
float4 PSMain(VS_OUTPUT _In) : SV_TARGET
{
	return gTexture.Sample(gSampler, _In.texcoord) * CalcColor(_In);
}
