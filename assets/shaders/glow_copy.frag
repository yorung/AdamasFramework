precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;

uniform sampler2D sourceMap;

void main() {
	vec4 src = texture2D(sourceMap, vfCoord);
	gl_FragColor = src;
}
