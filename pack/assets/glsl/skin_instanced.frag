precision mediump float;
varying vec2 texcoord;
varying vec4 diffuse;
varying vec3 normal;
varying vec3 emissive;
uniform sampler2D s3;

vec4 CalcColor()
{
	vec4 d;
	d.xyz = clamp(dot(normalize(normal), normalize(vec3(0.0, 1.0, -1.0))), 0.0, 1.0) * diffuse.xyz + emissive;
	d.w = 1.0;
	return d;
}

void main()
{
	gl_FragColor = texture2D(s3, texcoord) * CalcColor();
	gl_FragColor.w = 1.0;
}
