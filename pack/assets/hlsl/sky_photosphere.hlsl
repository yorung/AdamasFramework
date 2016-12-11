cbuffer perObject : register(b1)
{
	row_major matrix invVP;
}

Texture2D gTexture : register(t0);
SamplerState samplerState : register(s0);

struct VsToPs
{
	float4 pos : SV_POSITION;
	float4 pos2 : POS2;
};

#define RSDEF "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL), CBV(b1), StaticSampler(s0)"

[RootSignature(RSDEF)]
VsToPs VSMain(uint id : SV_VertexID)
{
	VsToPs ret;
	ret.pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	ret.pos2 = ret.pos;
	return ret;
}

[RootSignature(RSDEF)]
float4 PSMain(VsToPs inp) : SV_Target
{
	float3 dir = normalize(mul(inp.pos2, invVP).xyz);
	float longitude = atan2(dir.x, dir.z) * (180 / 3.14159265f);
	float latitude = asin(dir.y) * (180 / 3.14159265f);
	return gTexture.Sample(samplerState, float2(longitude, latitude) / float2(360, -180) + 0.5);
}
