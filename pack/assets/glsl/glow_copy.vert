#version 310 es

in vec2 POSITION;
out vec2 vfPosition;
out vec2 vfCoord;

void main() {
	vfPosition = POSITION;
	gl_Position = vec4(vfPosition.xy, 0, 1);
	vfCoord = vfPosition * 0.5 + 0.5;
}
