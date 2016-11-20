precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;
uniform sampler2D s0, s1, s2, s3, s4, s5, s6;

void main()
{
	vec4 org = texture2D(s6, vfCoord);
	vec4 g0 = texture2D(s0, vfCoord);
	vec4 g1 = texture2D(s1, vfCoord);
	vec4 g2 = texture2D(s2, vfCoord);
	vec4 g3 = texture2D(s3, vfCoord);
	vec4 g4 = texture2D(s4, vfCoord);
	vec4 g5 = texture2D(s5, vfCoord);
	gl_FragColor = org + g0 + g1 + g2 + g3 + g4 + g5;
}
