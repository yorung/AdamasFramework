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

void main() {
	float vel = sin(log(length(position - mousePos)) * 10.0 - elapsedTime * 3.0) * 2.0 + 2.0 + mouseDown;
	vel *= 1.0;
	if (mouseDown == 0.0) {
		vel = 0.0;
	}

	vec2 coord[] = {
		vec2(texcoord.x, texcoord.y),
		vec2(texcoord.x, texcoord.y) + vec2(-1.0 / 512.0, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, 1.0 / 512.0),
		vec2(texcoord.x, texcoord.y) + vec2(1.0 / 512.0, 0),
		vec2(texcoord.x, texcoord.y) + vec2(0, -1.0 / 512.0)
	};
	float ave = 0.0;
	for (int i = 0; i < 5; i++) {
		ave += texture(lastHeightMap, coord[i]).x;
	}
	ave /= 5.0;
	ave += vel;

	fragColor = vec4(ave, vel, vel, 1.0);
}
