#define vec4 float4
#define vec3 float3
#define vec2 float2
#define mod fmod
#define fract frac
#define mix lerp

Texture2D texSrc : register(t0);
SamplerState samplerState : register(s0);
#define texture2D(tex,coord) tex.Sample(samplerState, coord)

void VSMain(out vec4 pos : SV_POSITION, out vec2 vfPosition : vfPosition, out vec2 vfCoord : vfCoord, uint id : SV_VertexID)
{
	pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	vfPosition = pos.xy;
	vfCoord = vfPosition * 0.5 + 0.5;
}

void PSMain(vec2 vfPosition : vfPosition, vec2 vfCoord : vfCoord, out float4 fragColor: SV_Target)
{
	vec4 src = texture2D(texSrc, vfCoord);
	fragColor = src.w;
}
