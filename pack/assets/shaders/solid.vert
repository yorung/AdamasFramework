precision highp float;
attribute vec2 POSITION;
attribute vec3 COLOR;
varying vec3 color;

void main() {
	gl_Position = vec4(POSITION.xy, 0, 1);
	color = COLOR;
}
