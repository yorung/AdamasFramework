cbuffer perObject : register(b0)
{
	row_major matrix invVP;
}

TextureCube texCube : register(t1);
SamplerState samplerState : register(s1);

struct VsToPs
{
	float4 pos : SV_POSITION;
	float3 dir : DIR;
};

#define RSDEF "CBV(b0), DescriptorTable(SRV(t1)), StaticSampler(s1)"

[RootSignature(RSDEF)]
VsToPs VSMain(uint id : SV_VertexID)
{
	VsToPs ret;
	ret.pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	ret.dir = normalize(mul(ret.pos, invVP)).xyz;
	return ret;
}

[RootSignature(RSDEF)]
float4 PSMain(VsToPs inp) : SV_Target
{
	return texCube.Sample(samplerState, inp.dir);
}
