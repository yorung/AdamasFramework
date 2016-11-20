#version 310 es

precision highp float;
in vec2 vfPosition;
uniform vec4 b1[4];
uniform sampler2D s0;
layout (location = 0) out vec4 fragColor;

void main()
{
	mat4 invVP = mat4(b1[0], b1[1], b1[2], b1[3]);
	const float toDeg = 180.0 / 3.141592650;
	vec3 dir = normalize((invVP * vec4(vfPosition, 0.0, 1.0)).xyz);
//	float longitude = atan(dir.z / dir.x) * toDeg;
//	float longitude = atan(dir.z, dir.x) * toDeg;
	float longitude = atan(dir.x, dir.z) * toDeg;
	float latitude = asin(dir.y) * toDeg;
	fragColor = texture(s0, vec2(longitude, latitude) / vec2(360.0, -180.0) + 0.5);
//	fragColor = vec4((vec2(longitude, latitude) / vec2(360.0, -180.0) + 0.5), 1.0, 1.0);
//	fragColor = texture(s0, vfPosition);
//	fragColor = invVP[3];
//	fragColor = vec4(dir, 1.0);
}
