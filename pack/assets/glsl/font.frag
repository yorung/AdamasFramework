precision lowp float;
varying vec2 texcoord;
varying vec4 color;
uniform sampler2D texture;

void main()
{
	gl_FragColor = vec4(color.rgb, color.a * texture2D(texture, texcoord).a);
}
