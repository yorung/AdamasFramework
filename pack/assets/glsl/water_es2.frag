#ifdef GL_ES
precision mediump float;
#endif
varying vec2 position;
varying vec3 normal;
varying vec2 texcoord;
varying vec4 color;
uniform sampler2D s0, s1, s2, s3, s4, s5;
uniform vec4 b6[13];

const float loopTime = 20.0;
const float PI2 = 3.1415926 * 2.0;

void main()
{
	float time = b6[12].x;

	float dist1 = length(position + vec2(0.5, 0.5));
	float dist2 = length(position - vec2(0.5, 0.5));

	float radTimeUnit = time / loopTime * PI2;
	vec2 coord = vec2(texcoord.x, texcoord.y + sin(dist1 * 8.7 + radTimeUnit * 25.0) / 800.0 + sin(dist2 * 10.0 + radTimeUnit * 48.0) / 800.0);
//	vec2 coord = texcoord;

	vec4 c1 = texture2D(s0, coord);
	vec4 c2 = texture2D(s1, coord);
	vec4 c3 = texture2D(s2, coord);
	float delaymap = texture2D(s4, texcoord).x;
	vec4 timeline = texture2D(s3, vec2((time - delaymap) / loopTime, 0));
	vec4 bg = c1 * timeline.x + c2 * timeline.y + c3 * timeline.z;

//	vec3 normalForSample = cross(normal, vec3(1, 0, 0));
	vec3 normalForSample = normal;
	vec4 skyColor = texture2D(s5, normalForSample.xy * vec2(0.5, -0.5) + vec2(0.5, 0.5));
	gl_FragColor = mix(bg, skyColor * 1.5 + color, color.w);
}
