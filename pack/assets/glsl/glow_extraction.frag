#ifdef GL_ES
precision mediump float;
#endif
varying vec2 vfPosition;
varying vec2 vfCoord;
uniform sampler2D s0;

void main()
{
	vec4 src = texture2D(s0, vfCoord);
	float result = src.w;
	gl_FragColor = vec4(result);
}
