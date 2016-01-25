precision highp float;
varying vec2 vfPosition;
varying vec2 vfCoord;

uniform sampler2D sourceMap;

void main()
{
	vec4 src = texture2D(sourceMap, vfCoord);
	float result = src.w;
	gl_FragColor = vec4(result);
//	fragColor = src;
//	fragColor = vec4(vfCoord, 1.0, 1.0);
//	fragColor = vec4(vfPosition, 1.0, 1.0);
}
