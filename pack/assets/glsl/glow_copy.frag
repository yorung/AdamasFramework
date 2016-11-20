precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;
uniform sampler2D s0;

void main()
{
	gl_FragColor = texture2D(s0, vfCoord);
}
