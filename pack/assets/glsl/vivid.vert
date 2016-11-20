attribute vec2 POSITION;
varying vec2 texcoord;
varying vec2 position;

void main()
{
	gl_Position = vec4(POSITION.xy, 0, 1);
	texcoord.x = POSITION.x * 0.5 + 0.5;
	texcoord.y = POSITION.y * 0.5 + 0.5;
	position = POSITION;
}
