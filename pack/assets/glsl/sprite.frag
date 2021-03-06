#ifdef GL_ES
precision mediump float;
#endif
varying vec2 texcoord;
varying vec4 color;
uniform sampler2D s0;

void main()
{
	gl_FragColor = texture2D(s0, texcoord) * color;
}
