#version 310 es

precision highp float;
in vec2 vfPosition;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
layout (binding = 0) uniform sampler2D waterHeightmap;

vec2 mousePos = fakeUBO[0].xy;
float mouseDown = fakeUBO[0].z;
float elapsedTime = fakeUBO[1].x;
vec2 heightMapSize = fakeUBO[1].zw;

const float heightLimit = 0.4f;

vec2 DecodeWaterHeight(vec2 coord)
{
	vec4 t = texture(waterHeightmap, coord);
	return t.xy + t.zw / 256.0 - 0.5;
}

void main() {
	vec2 texcoord = vfPosition * 0.5 + 0.5;

	vec2 center = DecodeWaterHeight(texcoord);
	float height = center.x;
	float velocity = center.y;
	vec2 ofs[] = vec2[](
		texcoord + vec2(-1.0 / heightMapSize.x, 0),
		texcoord + vec2(0, 1.0 / heightMapSize.y),
		texcoord + vec2(1.0 / heightMapSize.x, 0),
		texcoord + vec2(0, -1.0 / heightMapSize.y)
	);
	float ave = 0.0;
	for (int i = 0; i < 4; i++) {
		vec2 o = ofs[i];
		float isCoordIn0To1 = mod(floor(o.x) + floor(o.y) + 1.0, 2.0);
		ave += DecodeWaterHeight(ofs[i]).x * isCoordIn0To1;
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

	vec2 hv = vec2(height, velocity) + 0.5;
	vec2 hvff = hv * 256.0;
	vec2 hvfract = fract(hvff);

	fragColor = vec4((hvff - hvfract) / 256.0, hvfract);
}
