#version 310 es

precision mediump float;
in vec2 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform sampler2D sampler2;
uniform sampler2D sampler3;
uniform sampler2D sampler4;
uniform sampler2D sampler5;
uniform sampler2D waterHeightmap;
uniform float time;

const float loopTime = 20.0;
const float PI2 = 3.1415926 * 2.0;

layout (location = 0) out vec4 fragColor;

void main() {
	float dist1 = length(position + vec2(0.5, 0.5));
	float dist2 = length(position - vec2(0.5, 0.5));

	float radTimeUnit = time / loopTime * PI2;
	vec2 coord = vec2(texcoord.x, texcoord.y + sin(dist1 * 8.7 + radTimeUnit * 25.0) / 800.0 + sin(dist2 * 10.0 + radTimeUnit * 48.0) / 800.0);
//	vec2 coord = texcoord;

	vec4 c1 = texture(sampler0, coord);
	vec4 c2 = texture(sampler1, coord);
	vec4 c3 = texture(sampler2, coord);
	float delaymap = texture(sampler4, texcoord).x;
	vec4 timeline = texture(sampler3, vec2((time - delaymap) / loopTime, 0));
	vec4 bg = c1 * timeline.x + c2 * timeline.y + c3 * timeline.z;

	vec4 height = texture(waterHeightmap, texcoord);

//	vec3 normalForSample = cross(normal, vec3(1, 0, 0));
	vec3 normalForSample = normal;
	vec4 skyColor = texture(sampler5, normalForSample.xy * vec2(0.5, -0.5) + vec2(0.5, 0.5));
//	fragColor = mix(bg, skyColor * 3.0, color.w);
	fragColor = mix(bg, skyColor * 1.5 + color, color.w) * height;
//	fragColor = skyColor;
}
