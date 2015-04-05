precision lowp float;
varying vec2 texcoord;
varying vec4 color;
uniform sampler2D sampler;

void main() {
//	gl_FragColor = color;
	gl_FragColor = texture2D(sampler, texcoord) * color;
	gl_FragColor.w = 1.0;
//	gl_FragColor = vec4(1, 1, 1, 1);
}

