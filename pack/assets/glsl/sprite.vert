attribute vec3 POSITION;
attribute vec2 TEXCOORD;
attribute vec4 COLOR;
varying vec2 texcoord;
varying vec4 color;
uniform vec4 b1[4];

void main()
{
	mat4 matProj = mat4(b1[0], b1[1], b1[2], b1[3]);
	gl_Position = matProj * vec4(POSITION.xyz, 1);
	texcoord = TEXCOORD;
	color = COLOR;
}
