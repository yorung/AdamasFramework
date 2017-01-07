#version 450

layout (location = 0) in vec2 vfPosition;

layout (std140, set = 0, binding = 0) uniform uniformBuf
{
		vec4 b0;
};

layout (set = 1, binding = 0) uniform sampler2D s1;

layout (location = 0) out vec4 fragColor;

float GetWaterHeight(vec2 position)
{
	vec2 coord = position * 0.5 + 0.5;
	return texture(s1, coord).x;
}

vec3 MakeWater3DPos(vec2 position)
{
	return vec3(position, GetWaterHeight(position));
}

vec3 GetSurfaceNormal()
{
	vec3 heightU = MakeWater3DPos(vfPosition + vec2(0, 1.0 / (b0.y * 0.5)));
	vec3 heightL = MakeWater3DPos(vfPosition - vec2(1.0 / (b0.x * 0.5), 0));
	vec3 height = MakeWater3DPos(vfPosition);
	vec3 normalFromHeightMap = cross(heightU - height, heightL - height);

	return normalize(normalFromHeightMap);
}

void main()
{
//	vec2 normXYEncoded = GetSurfaceNormal().xy * 0.5 + 0.5;
	vec2 normXYEncoded = pow(GetSurfaceNormal().xy * 0.5 + 0.5, vec2(1.0 / 2.0));
//	vec2 normXYEncoded = sin((GetSurfaceNormal().xy * 0.5 + 0.5) * (3.1415926 / 2.0));

	vec2 normXY256 = normXYEncoded * 256.0;
	vec2 xyFract = fract(normXY256);
	fragColor.xy = (normXY256 - xyFract) / 256.0;
	fragColor.zw = xyFract;
}
