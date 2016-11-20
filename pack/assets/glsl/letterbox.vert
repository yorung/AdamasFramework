attribute vec2 POSITION;
varying vec2 vfPosition;
varying vec2 vfCoord;

void main()
{
	vfPosition = POSITION;
	gl_Position = vec4(vfPosition.xy, 0, 1);
	vfCoord = vfPosition * 0.5 + 0.5;
}
