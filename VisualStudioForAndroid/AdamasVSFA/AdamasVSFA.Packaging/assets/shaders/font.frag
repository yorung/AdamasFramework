precision lowp float;
varying vec2 texcoord;
uniform sampler2D texture;

void main() {
	gl_FragColor = texture2D(texture, texcoord);
//	gl_FragColor = vec4(texture2D(texture, texcoord).xyz, 1.0);
//	gl_FragColor = vec4(0.5, 0.5, 1.0, 1.0) + texture2D(texture, texcoord);
}
