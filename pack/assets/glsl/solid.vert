precision highp float;
attribute vec3 POSITION;
attribute vec3 COLOR;
varying vec3 color;

uniform vec4 b0[4];

void main()
{
	mat4 matPV = mat4(b0[0], b0[1], b0[2], b0[3]);
	gl_Position = matPV * vec4(POSITION, 1);
	color = COLOR;
}
