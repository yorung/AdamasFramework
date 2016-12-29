#define vec4 float4
#define vec3 float3
#define vec2 float2
#define mod fmod

cbuffer uniformBuffer : register(b0)
{
	vec2 mousePos;
	float mouseDown;
	float padding;
	float elapsedTime;
	float wrappedTime;
	vec2 heightMapSize;
}

Texture2D lastHeightMap : register(t0);
SamplerState samplerState : register(s0);
#define texture(tex,coord) tex.Sample(samplerState, vec2(coord.x, 1.0 - coord.y))

static const float heightLimit = 0.4f;

void VSMain(out float4 pos : SV_POSITION, out vec2 vfPosition : vfPosition, uint id : SV_VertexID)
{
	pos = float4(id & 2 ? 1 : -1, id & 1 ? -1 : 1, 1, 1);
	vfPosition = pos.xy;
}

void PSMain(float4 pos : SV_POSITION, vec2 vfPosition : vfPosition, out float4 fragColor: SV_Target)
{
	vec2 texcoord = vfPosition * 0.5 + 0.5;

	vec4 center = texture(lastHeightMap, texcoord);
	float height = center.x;
	float velocity = center.y;
	vec2 ofs[] = {
		texcoord + vec2(-1.0 / heightMapSize.x, 0),
		texcoord + vec2(0, 1.0 / heightMapSize.y),
		texcoord + vec2(1.0 / heightMapSize.x, 0),
		texcoord + vec2(0, -1.0 / heightMapSize.y)
	};
	float ave = 0.0;
	for (int i = 0; i < 4; i++) {
		vec2 o = ofs[i];
		float isCoordIn0To1 = mod(floor(o.x) + floor(o.y) + 1.0, 2.0);
		ave += texture(lastHeightMap, ofs[i]).x * isCoordIn0To1;
	}
	ave /= 4.0;
	float velAdd = ave - height;

	float dist = length(vfPosition - mousePos);
	height += pow(max(0.0, 1.0 - dist * 9.0), 3.0) * 0.015f * mouseDown;
//	height += max(0.0f, 1.0f - dist * 9.0) * 0.015f * mouseDown;
//	height = clamp(height, -heightLimit, heightLimit);

	velocity += velAdd;
	velocity *= 0.99;
	height += velocity;

	fragColor = vec4(height, velocity, 0.0, 1.0);
}
