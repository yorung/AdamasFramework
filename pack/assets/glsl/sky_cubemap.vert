attribute vec2 POSITION;
varying vec2 vfPosition;

void main()
{
	gl_Position = vec4(POSITION.xy, -1, 1);
	vfPosition = gl_Position.xy;
}
