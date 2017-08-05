varying vec2 vfPosition;
varying vec2 vfCoord;
uniform sampler2D s0;

void main()
{
	vec2 coord = vfPosition * 0.5 + 0.5;
	float brightness = vfPosition.y < -0.8 || vfPosition.y > 0.8 ? 0.1 : 1.0;
	gl_FragColor = vec4(texture2D(s0, coord).xyz * brightness, 1.0);
}
