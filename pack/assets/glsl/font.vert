precision highp float;
attribute vec2 POSITION;
attribute vec2 TEXCOORD;
attribute vec4 COLOR;
varying vec2 texcoord;
varying vec4 color;

void main()
{
	gl_Position = vec4(POSITION.xy * vec2(1, -1), 0, 1);
	texcoord = TEXCOORD;
	color = COLOR;
}
