#version 310 es

precision highp float;

in vec2 vfPosition;

layout (std140, binding = 1, row_major) uniform inst {
	mat4 invVP;
};

layout (binding = 0) uniform sampler2D sampler;
layout (location = 0) out vec4 fragColor;

void main()
{
	vec2 scale = vec2(4.0f / 3.0f, 1.0) * 2.0;	// scale & aspect ratio
	vec2 plane = vfPosition * scale;
	vec3 dir = vec3(plane.x * 2.0, plane.y * 2.0, -1.0 + dot(plane, plane)) / (1.0 + dot(plane, plane));

	dir = dir.xzy;	// y is upper

	float longitude = atan(dir.x, dir.z) * (180.f / 3.14159265f);
	float latitude = asin(dir.y) * (180.f / 3.14159265f);
	fragColor = texture(sampler, vec2(longitude, latitude) / vec2(360.0, -180.0) + 0.5);
}
