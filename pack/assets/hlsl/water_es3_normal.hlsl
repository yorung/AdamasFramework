#define vec4 float4
#define vec3 float3
#define vec2 float2
#define mod fmod
#define fract frac

Texture2D waterHeightmap : register(t0);
SamplerState samplerState : register(s0);
#define texture(tex,coord) tex.Sample(samplerState, vec2(coord.x, 1.0 - coord.y))

cbuffer uniformBuffer : register(b0)
{
	vec4 heightMapSize;
}

float GetWaterHeight(vec2 position)
{
	vec2 coord = position * 0.5 + 0.5;
	return texture(waterHeightmap, coord).x;
}

vec3 MakeWater3DPos(vec2 position)
{
	return vec3(position, GetWaterHeight(position));
}

vec3 GetSurfaceNormal(vec2 vfPosition)
{
	vec3 heightU = MakeWater3DPos(vfPosition + vec2(0, 1.0 / (heightMapSize.y * 0.5)));
	vec3 heightL = MakeWater3DPos(vfPosition - vec2(1.0 / (heightMapSize.x * 0.5), 0));
	vec3 height = MakeWater3DPos(vfPosition);
	vec3 normalFromHeightMap = cross(heightU - height, heightL - height);

	return normalize(normalFromHeightMap);
}

void VSMain(out float4 pos : SV_POSITION, out vec2 vfPosition : vfPosition, uint id : SV_VertexID)
{
	pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	vfPosition = pos.xy;
}

void PSMain(float4 pos : SV_POSITION, vec2 vfPosition : vfPosition, out float4 fragColor: SV_Target)
{
	vec2 normXYEncoded = pow(GetSurfaceNormal(vfPosition).xy * 0.5 + 0.5, vec2(1.0 / 2.0, 1.0 / 2.0));
	vec2 normXY256 = normXYEncoded * 256.0;
	vec2 xyFract = fract(normXY256);
	fragColor.xy = (normXY256 - xyFract) / 256.0;
	fragColor.zw = xyFract;
}
