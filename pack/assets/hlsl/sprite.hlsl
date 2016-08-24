struct VertIn {
	float3 POSITION : POSITION;
	float2 TEXCOORD : TEXCOORD;
	float4 COLOR : COLOR;
};

struct VsToPs {
	float4 SV_POSITION : SV_POSITION;
	float2 texcoord : texcoord;
	float4 color : color;
};

cbuffer matUbo : register(b0) {
	matrix matProj;
};

#define RSDEF "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), CBV(b0), DescriptorTable(SRV(t0)), StaticSampler(s0)"

[RootSignature(RSDEF)]
VsToPs VSMain(VertIn _In)
{
	VsToPs o;
	o.SV_POSITION = mul(matProj, float4(_In.POSITION.xyz, 1));
	o.texcoord = _In.TEXCOORD;
	o.color = _In.COLOR;
	return o;
}

SamplerState psSampler : register(s0);
Texture2D psTexture : register(t0);

[RootSignature(RSDEF)]
float4 PSMain(VsToPs _In) : SV_TARGET
{
	return psTexture.Sample(psSampler, _In.texcoord) * _In.color;
}
