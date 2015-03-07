precision mediump float;
varying vec2 position;
varying vec2 texcoord;
uniform sampler2D sampler;
void main() {
	vec2 coord = vec2(texcoord.x, texcoord.y);
	float brightness = position.y < -0.8f || position.y > 0.8f ? 0.1f : 1.0f;
	gl_FragColor = vec4(texture2D(sampler, coord).xyz * brightness, 1.0);
}
