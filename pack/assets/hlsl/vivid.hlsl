Texture2D gTexture : register(t0);
SamplerState samplerState : register(s0);

struct VsToPs
{
	float4 pos : SV_POSITION;
	float4 screenPos : POS2;
};

VsToPs mainVS(uint id : SV_VertexID)
{
	VsToPs ret;
	ret.pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	ret.screenPos = ret.pos;
	return ret;
}

float4 mainPS(VsToPs inp) : SV_Target
{
	return gTexture.Sample(samplerState, inp.screenPos.xy * float2(0.5, -0.5) + 0.5) * 2;
}