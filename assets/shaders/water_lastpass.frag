#version 310 es

precision highp float;
in vec2 vfPosition;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform sampler2D sampler2;
uniform sampler2D sampler3;
uniform sampler2D sampler4;
uniform sampler2D sampler5;
uniform sampler2D waterHeightmap;

layout (location = 0) uniform vec4 fakeUBO[2];

float wrappedTime = fakeUBO[1].y;
vec2 heightMapSize = fakeUBO[1].zw;

const float loopTime = 20.0;
const float PI2 = 3.1415926 * 2.0;

const float airToWater = 1.0 / 1.33333;

const vec3 invGamma3 = vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);
const vec3 gamma3 = vec3(2.2, 2.2, 2.2);

const vec3 camDir = vec3(0, 0, -1);
const float waterDepth = 0.1;

layout (location = 0) out vec4 fragColor;

vec4 FetchWaterTex(vec2 position)
{
	vec2 coord = position * 0.5 + 0.5;
	return texture(waterHeightmap, coord);
}

vec3 MakeWater3DPos(vec2 position)
{
	return vec3(position, FetchWaterTex(position).x);
}

vec3 GetSurfaceNormal(vec2 position)
{
	vec4 h = FetchWaterTex(position);
	vec3 normal = vec3(h.zw, sqrt(1.0 - dot(h.zw, h.zw)));
	return normal;
/*
	vec3 heightU = MakeWater3DPos(vfPosition + vec2(0, 1.0 / (heightMapSize.y * 0.5)));
	vec3 heightL = MakeWater3DPos(vfPosition - vec2(1.0 / (heightMapSize.x * 0.5), 0));
	vec3 height = MakeWater3DPos(vfPosition);
	vec3 normalFromHeightMap = cross(heightU - height, heightL - height);

	return normalize(normalFromHeightMap);
*/
}

float GetFakeSunIllum(vec2 position, vec3 normal)
{
	const vec3 lightPos = vec3(0.5, 0.5, 4.0);
	vec3 lightDir = normalize(lightPos - vec3(position, 0));
	vec3 eyeDir = vec3(0, 0, 1);
	vec3 reflectedRay = reflect(-eyeDir, normal);
	return pow(max(0.0, dot(lightDir, reflectedRay)), 2000.0);
}

vec3 IntersectRayWithBottom(vec2 surfacePos, vec3 surfaceNormal)
{
	const vec3 lightPos = vec3(0.5, 0.5, 4.0);
	vec3 pos3d = vec3(surfacePos, 0);
	vec3 lightDir = normalize(lightPos - pos3d);
	vec3 eyeDir = vec3(0, 0, 1);
	vec3 refractedRay = refract(-eyeDir, surfaceNormal, airToWater);
	return pos3d + refractedRay * waterDepth;
}

float GetCaustics(vec2 position)
{
	vec2 ofsY = position + vec2(0, 1.0 / (heightMapSize.y * 0.5));
	vec2 ofsX = position + vec2(1.0 / (heightMapSize.x * 0.5), 0);

	vec3 resultCenter = IntersectRayWithBottom(position, GetSurfaceNormal(position));
	vec3 resultOfsX = IntersectRayWithBottom(ofsX, GetSurfaceNormal(ofsX));
	vec3 resultOfsY = IntersectRayWithBottom(ofsY, GetSurfaceNormal(ofsY));

	vec3 resultOrgCenter = IntersectRayWithBottom(position, vec3(0.0, 0.0, 1.0));
	vec3 resultOrgOfsX = IntersectRayWithBottom(ofsX, vec3(0.0, 0.0, 1.0));
	vec3 resultOrgOfsY = IntersectRayWithBottom(ofsY, vec3(0.0, 0.0, 1.0));

	float sqWithWave = length(resultOfsX - resultCenter) * length(resultOfsY - resultCenter);
	float sqOrg = length(resultOrgOfsX - resultOrgCenter) * length(resultOrgOfsY - resultOrgCenter);

	return sqOrg / sqWithWave;
//	return sqOrg;
//	return 1.0;
}

float GetCaustics1(vec2 position)
{
	vec3 normal = GetSurfaceNormal(position);
	const vec3 lightPos = vec3(0.5, 0.5, 4.0);
	vec3 lightDir = normalize(lightPos - vec3(position, 0));
	vec3 eyeDir = vec3(0, 0, 1);
	vec3 refractedRay = refract(-eyeDir, normal, airToWater);
	vec3 refractedRayOrg = refract(-eyeDir, vec3(0.0, 0.0, 1.0), airToWater);
	return dot(refractedRay, refractedRayOrg);
}

vec3 GetBGColor(vec2 coord)
{
	vec3 c1 = texture(sampler0, coord).xyz;
	vec3 c2 = texture(sampler1, coord).xyz;
	vec3 c3 = texture(sampler2, coord).xyz;
	float delaymap = texture(sampler4, coord).x;
	vec4 timeline = texture(sampler3, vec2((wrappedTime - delaymap) / loopTime, 0));
	vec3 bg = c1 * timeline.x + c2 * timeline.y + c3 * timeline.z;
	return bg;
}

void main() {
	fragColor.w = 1.0;

	vec3 normal = GetSurfaceNormal(vfPosition);
//	vec3 rayDirOrg = refract(camDir, vec3(0.0, 1.0, 0.0), airToWater);
	vec3 rayDir = refract(camDir, normal, airToWater);
	vec3 bottom = rayDir * waterDepth / rayDir.z;
	vec2 texcoord = (vfPosition.xy + bottom.xy) * vec2(0.5, -0.5) + vec2(0.5, 0.5);

	vec3 bg = GetBGColor(texcoord);

	vec3 skyColor = texture(sampler5, normal.xy * vec2(0.5, -0.5) + vec2(0.5, 0.5)).xyz;

	// gamma -> linear
	bg = pow(bg, invGamma3) * 0.5;
	skyColor = pow(skyColor, invGamma3) * 0.5;

	float sunStr = GetFakeSunIllum(vfPosition.xy, normal);

	float mixFactor = 1.0 - dot(normal, vec3(0, 0, 1));
//	vec3 outCol = mix(bg, min(vec3(5.5), vec3(1.0, 1.0, 1.0) * 50.5), mixFactor) + sunStr;
//	vec3 outCol = mix(bg, skyColor * 50.5, mixFactor) + sunStr;
	vec3 outCol = mix(bg, skyColor * 0.5, mixFactor) * (GetCaustics(vfPosition) + 0.3) + sunStr;

//	fragColor.xyz = height.zzz;
//	fragColor.xyz = 0.5 + normalFromHeightMap;

// ok
//	fragColor.xyz = normal;

// ok
//	fragColor.xyz = height.zzz;

//wrong color when mediump
// 	fragColor = bg;

	// linear -> gamma
	fragColor.rgb = pow(outCol * 1.5, gamma3);

//	fragColor.rgb = IntersectRayWithBottom(vfPosition, GetSurfaceNormal(vfPosition));
}
