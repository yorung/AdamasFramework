#version 310 es

precision mediump float;
in vec2 position;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform vec4 fakeUBO[2];
layout (binding = 0) uniform sampler2D lastHeightMap;

vec2 mousePos = fakeUBO[0].xy;
float mouseDown = fakeUBO[0].z;
float elapsedTime = fakeUBO[0].w;

const float texW = 512.0;
const float texH = 512.0;

void main() {
	vec4 center = texture(lastHeightMap, vec2(texcoord.x, texcoord.y));
	vec2 ofs[] = {
		vec2(texcoord.x, texcoord.y) + vec2(-1.0 / texW, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, 1.0 / texH),
		vec2(texcoord.x, texcoord.y) + vec2(1.0 / texW, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, -1.0 / texH)
	};
	float ave = 0.0;
	for (int i = 0; i < 4; i++) {
		ave += texture(lastHeightMap, ofs[i]).x;
	}
	ave /= 4.0;
	float vel = ave - center.x;

	if (mouseDown != 0.0) {
		float velUser = length(position - mousePos) < 0.05 ? 0.5 : 0.0;
		velUser *= 100.0;
		vel += velUser;
	}

	center.y += vel;
	center.y *= 0.99;
	center.x += center.y;

	fragColor = center;
}
