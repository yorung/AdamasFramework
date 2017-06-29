Texture2D tex : register(t0);

struct VsToPs
{
	float4 pos : SV_POSITION;
	float4 screenPos : POS2;
};

#define RSDEF "DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL)"

[RootSignature(RSDEF)]
VsToPs VSMain(uint id : SV_VertexID)
{
	VsToPs ret;
	ret.pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	ret.screenPos = ret.pos;
	return ret;
}

[RootSignature(RSDEF)]
float4 PSMain(VsToPs inp) : SV_Target
{
	return tex.Load(float3(inp.pos.xy, 0));
}
