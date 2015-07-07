attribute vec2 POSITION;
varying vec2 vfPosition;

void main() {
	gl_Position = vec4(POSITION, 0, 1);
	vfPosition = POSITION;
}
