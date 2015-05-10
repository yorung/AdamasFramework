#version 310 es

precision mediump float;
out vec2 vfPosition;
out vec3 vfNormal;
uniform sampler2D waterHeightmap;

layout (location = 0) uniform vec4 fakeUBO[2];

vec2 heightMapSize = fakeUBO[1].zw;

layout (location = 2) in vec2 vCoord;
in int gl_VertexID;

vec4 FetchWaterTex(vec2 position)
{
	vec2 coord = position * 0.5 + 0.5;
	return texture(waterHeightmap, coord);
}

vec3 MakeWater3DPos(vec2 position)
{
	return vec3(position, FetchWaterTex(position).x * 0.8);
}

void main() {
	gl_Position = vec4(vCoord, 0, 1);
	vec3 heightU = MakeWater3DPos(vCoord + vec2(0, 1.0 / (heightMapSize.y * 0.5)));
	vec3 heightL = MakeWater3DPos(vCoord - vec2(1.0 / (heightMapSize.x * 0.5), 0));
	vec3 height = MakeWater3DPos(vCoord);
	vfPosition = vCoord;
	vfNormal = normalize(cross(heightU - height, heightL - height));
}
