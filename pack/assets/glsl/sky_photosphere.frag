#version 310 es

precision highp float;
in vec2 vfPosition;

layout (location = 0) out vec4 fragColor;
layout (std140, binding = 0, row_major) uniform inst {
	mat4 invVP;
};
layout (binding = 1) uniform sampler2D tex;

void main() {
	const float toDeg = 180.0 / 3.141592650;
	vec3 dir = normalize((vec4(vfPosition, 0.0, 1.0) * invVP).xyz);
//	float longitude = atan(dir.z / dir.x) * toDeg;
//	float longitude = atan(dir.z, dir.x) * toDeg;
	float longitude = atan(dir.x, dir.z) * toDeg;
	float latitude = asin(dir.y) * toDeg;
	fragColor = texture(tex, vec2(longitude, latitude) / vec2(360.0, -180.0) + 0.5);
//	fragColor = vec4((vec2(longitude, latitude) / vec2(360.0, -180.0) + 0.5), 1.0, 1.0);
//	fragColor = texture(tex, vfPosition);
//	fragColor = invVP[3];
//	fragColor = vec4(dir, 1.0);
}
