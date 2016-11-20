precision mediump float;
varying vec2 position;
varying vec2 texcoord;
uniform sampler2D s0;

void main()
{
	vec2 coord = vec2(texcoord.x, texcoord.y);
	gl_FragColor = vec4(texture2D(s0, coord).xyz * 2.0, 1.0);
}
