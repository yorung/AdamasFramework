precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;

uniform sampler2D glow0;
uniform sampler2D glow1;
uniform sampler2D glow2;
uniform sampler2D glow3;
uniform sampler2D glow4;
uniform sampler2D glow5;
uniform sampler2D org;

void main() {
	vec4 o = texture2D(org, vfCoord);
	vec4 g0 = texture2D(glow0, vfCoord);
	vec4 g1 = texture2D(glow1, vfCoord);
	vec4 g2 = texture2D(glow2, vfCoord);
	vec4 g3 = texture2D(glow3, vfCoord);
	vec4 g4 = texture2D(glow4, vfCoord);
	vec4 g5 = texture2D(glow5, vfCoord);
//	float result = src.w;
//	fragColor = vec4(result);

	gl_FragColor = o + g0 + g1 + g2 + g3 + g4 + g5;
}
