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
const vec3 camDir = vec3(0, 0, -1);
const float waterDepth = 0.2;

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

void main_() {
	fragColor = vec4(1, 1, 0, 1);
}


void main() {
	vec2 position = vfPosition;
	vec3 heightU = MakeWater3DPos(position + vec2(0, 1.0 / (heightMapSize.y * 0.5)));
	vec3 heightL = MakeWater3DPos(position - vec2(1.0 / (heightMapSize.x * 0.5), 0));
	vec3 height = MakeWater3DPos(position);
	vec3 normalFromHeightMap = cross(heightU - height, heightL - height);

	vec3 normal = normalize(normalFromHeightMap);
//	vec3 normal = normalize(vfNormal);

	vec3 rayDir = refract(camDir, normal, airToWater);
	vec3 bottom = rayDir * waterDepth / rayDir.z;
	vec2 texcoord = (position.xy + bottom.xy) * vec2(0.5, -0.5) + vec2(0.5, 0.5);
	float mask = dot(normal, vec3(0, 0, 1));
	vec4 color = vec4(1, 1, 1, 1.0 - mask);


	float dist1 = length(position + vec2(0.5, 0.5));
	float dist2 = length(position - vec2(0.5, 0.5));

	float radTimeUnit = wrappedTime / loopTime * PI2;
//	vec2 coord = vec2(texcoord.x, texcoord.y + sin(dist1 * 8.7 + radTimeUnit * 25.0) / 800.0 + sin(dist2 * 10.0 + radTimeUnit * 48.0) / 800.0);
	vec2 coord = texcoord;

	vec4 c1 = texture(sampler0, coord);
	vec4 c2 = texture(sampler1, coord);
	vec4 c3 = texture(sampler2, coord);
	float delaymap = texture(sampler4, texcoord).x;
	vec4 timeline = texture(sampler3, vec2((wrappedTime - delaymap) / loopTime, 0));
	vec4 bg = c1 * timeline.x + c2 * timeline.y + c3 * timeline.z;
//	vec4 bg = vec4(normal, 1.0);


	vec3 normalForSample = normal;
	vec4 skyColor = texture(sampler5, normalForSample.xy * vec2(0.5, -0.5) + vec2(0.5, 0.5));
	fragColor = mix(bg, skyColor * 1.5 + color, color.w);
//	fragColor.xyz = height.zzz;
//	fragColor.xyz = 0.5 + normalFromHeightMap;

// ok
//	fragColor.xyz = normal;

// ok
//	fragColor.xyz = height.zzz;

//wrong color when mediump
// 	fragColor = bg;

}
