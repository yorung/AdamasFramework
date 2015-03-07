precision mediump float;
varying vec2 position;
varying vec2 texcoord;
uniform sampler2D sampler;
void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	float brightness = position.y < -0.8 || position.y > 0.8 ? 0.1 : 1.0;
	gl_FragColor = vec4(texture2D(sampler, coord).xyz * brightness, 1.0);
}
