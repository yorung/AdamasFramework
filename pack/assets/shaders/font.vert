precision highp float;
attribute vec2 POSITION;
attribute vec2 TEXCOORD;
varying vec2 texcoord;

void main() {
	gl_Position = vec4(POSITION.xy, 0, 1);
	texcoord = TEXCOORD;
}
