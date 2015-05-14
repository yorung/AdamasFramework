#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
layout (binding = 0) uniform sampler2D lastHeightMap;

vec2 mousePos = fakeUBO[0].xy;
float mouseDown = fakeUBO[0].z;
float elapsedTime = fakeUBO[1].x;
vec2 heightMapSize = fakeUBO[1].zw;

const float heightLimit = 0.4f;

void main() {
	vec4 center = texture(lastHeightMap, vec2(texcoord.x, texcoord.y));
	vec2 ofs[] = {
		vec2(texcoord.x, texcoord.y) + vec2(-1.0 / heightMapSize.x, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, 1.0 / heightMapSize.y),
		vec2(texcoord.x, texcoord.y) + vec2(1.0 / heightMapSize.x, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, -1.0 / heightMapSize.y)
	};
	float ave = 0.0;
	for (int i = 0; i < 4; i++) {
		ave += texture(lastHeightMap, ofs[i]).x;
	}
	ave /= 4.0;
	float vel = ave - center.x;

	if (mouseDown != 0.0) {
		float velUser = length(position - mousePos) < 0.05 ? 0.5 : 0.0;
		velUser *= 1.0;
		vel += velUser;
	}

	float dist = length(position - mousePos);
	center.x += max(0.0f, 1.0f - dist * 9.0) * 0.015f * mouseDown;
	center.x = clamp(center.x, -heightLimit, heightLimit);

	center.y += vel;
	center.y *= 0.99;
	center.x += center.y;

	fragColor = center;
}
