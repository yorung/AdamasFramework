struct VsIn {
	float3 vPosition : vPosition;
	float3 vNormal : vNormal;
};

struct VsToPs {
	float2 texcoord : texcoord;
	float2 position : position;
	float3 normal : normal;
	float4 color : color;
	float4 pos4 : SV_POSITION;
};

cbuffer perDrawCallUBO : register(b0) {
	matrix matW;
	matrix matV;
	matrix matP;
	float time;
};

static const float airToWater = 1.0 / 1.33333;
static const float3 camDir = float3(0, 0, -1);
static const float waterDepth = 0.2;

VsToPs VSMain(VsIn vsIn)
{
	VsToPs vsToPs;

	matrix matWVP = mul(matP, mul(matV, matW));
	vsToPs.pos4 = mul(matWVP, float4(vsIn.vPosition.xyz, 1));
	vsToPs.normal = mul(normalize(vsIn.vNormal), (float3x3)matW);

	float3 rayDir = refract(camDir, vsToPs.normal, airToWater);

	float3 bottom = rayDir * waterDepth / rayDir.z;

	vsToPs.position = vsIn.vPosition.xz;
	vsToPs.texcoord = (vsIn.vPosition.xz + bottom.xy) * float2(0.5, -0.5) + float2(0.5, 0.5);

	float mask = dot(vsToPs.normal, float3(0, 0, 1));
	vsToPs.color = float4(1, 1, 1, 1.0 - mask);

	return vsToPs;
}

SamplerState sampler0 : register(s0);
SamplerState sampler1 : register(s1);
SamplerState sampler2 : register(s2);
SamplerState sampler3 : register(s3);
SamplerState sampler4 : register(s4);
SamplerState sampler5 : register(s5);
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
Texture2D texture4 : register(t4);
Texture2D texture5 : register(t5);

static const float loopTime = 20.0;
static const float PI2 = 3.1415926 * 2.0;

float4 PSMain(VsToPs psIn) : SV_TARGET
{
	float dist1 = length(psIn.position + float2(0.5, 0.5));
	float dist2 = length(psIn.position - float2(0.5, 0.5));

	float radTimeUnit = time / loopTime * PI2;
	float2 coord = float2(psIn.texcoord.x, psIn.texcoord.y + sin(dist1 * 8.7 + radTimeUnit * 25.0) / 800.0 + sin(dist2 * 10.0 + radTimeUnit * 48.0) / 800.0);
//	float2 coord = texcoord;

	float4 c1 = texture0.Sample(sampler0, coord);
	float4 c2 = texture1.Sample(sampler1, coord);
	float4 c3 = texture2.Sample(sampler2, coord);
	float delaymap = texture4.Sample(sampler4, psIn.texcoord).x;
	float4 timeline = texture3.Sample(sampler3, float2((time - delaymap) / loopTime, 0));
	float4 bg = c1 * timeline.x + c2 * timeline.y + c3 * timeline.z;

//	float3 normalForSample = cross(normal, float3(1, 0, 0));
	float3 normalForSample = psIn.normal;
	float4 skyColor = texture5.Sample(sampler5, normalForSample.xy * float2(0.5, -0.5) + float2(0.5, 0.5));
	return lerp(bg, skyColor * 1.5 + psIn.color, psIn.color.w);
}
