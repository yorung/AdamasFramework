#define vec4 float4
#define vec3 float3
#define vec2 float2
#define mod fmod
#define fract frac
#define mix lerp

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D tex3 : register(t3);
Texture2D tex4 : register(t4);
Texture2D tex5 : register(t5);
Texture2D tex6 : register(t6);
SamplerState samplerState : register(s0);
#define texture2D(tex,coord) tex.Sample(samplerState, vec2(coord.x, 1.0 - coord.y))

#define RSDEF \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),"\
	"DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t1), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t2), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t3), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t4), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t5), visibility=SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t6), visibility=SHADER_VISIBILITY_PIXEL),"\
	"StaticSampler(s0)"

[RootSignature(RSDEF)]
void VSMain(out vec2 vfPosition : vfPosition, out vec2 vfCoord : vfCoord, out vec4 pos : SV_POSITION, uint id : SV_VertexID)
{
	pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	vfPosition = pos.xy;
	vfCoord = vfPosition * 0.5 + 0.5;
}

[RootSignature(RSDEF)]
void PSMain(vec2 vfPosition : vfPosition, vec2 vfCoord : vfCoord, out float4 fragColor: SV_Target)
{
	vec4 org = texture2D(tex6, vfCoord);
	vec4 g0 = texture2D(tex0, vfCoord);
	vec4 g1 = texture2D(tex1, vfCoord);
	vec4 g2 = texture2D(tex2, vfCoord);
	vec4 g3 = texture2D(tex3, vfCoord);
	vec4 g4 = texture2D(tex4, vfCoord);
	vec4 g5 = texture2D(tex5, vfCoord);
	fragColor = org + g0 + g1 + g2 + g3 + g4 + g5;
}
