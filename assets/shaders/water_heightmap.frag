#version 310 es

precision highp float;
in vec2 vfPosition;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
layout (binding = 0) uniform sampler2D lastHeightMap;

vec2 mousePos = fakeUBO[0].xy;
float mouseDown = fakeUBO[0].z;
float elapsedTime = fakeUBO[1].x;
vec2 heightMapSize = fakeUBO[1].zw;

const float heightLimit = 0.4f;

void main() {
	vec2 texcoord = vfPosition * 0.5 + 0.5;

	vec4 center = texture(lastHeightMap, texcoord);
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
		ave += texture(lastHeightMap, ofs[i]) * isCoordIn0To1;
	}
	ave /= 4.0;
	float vel = ave - center.x;

	float dist = length(vfPosition - mousePos);
	center.x += pow(max(0.0, 1.0 - dist * 9.0), 3.0) * 0.015f * mouseDown;
//	center.x += max(0.0f, 1.0f - dist * 9.0) * 0.015f * mouseDown;
//	center.x = clamp(center.x, -heightLimit, heightLimit);

	center.y += vel;
	center.y *= 0.99;
	center.x += center.y;

	fragColor = center;
}
