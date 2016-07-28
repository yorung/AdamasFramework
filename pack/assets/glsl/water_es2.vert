#version 310 es

precision mediump float;
in vec3 vPosition;
in vec3 vNormal;
out vec2 texcoord;
out vec2 position;
out vec3 normal;
out vec4 color;

layout (std140, binding = 0) uniform matrices {
	mat4 matW;
	mat4 matV;
	mat4 matP;
	float time;
};

const float airToWater = 1.0 / 1.33333;
const vec3 camDir = vec3(0, 0, -1);
const float waterDepth = 0.2;

void main() {
	mat4 matWVP = matP * matV * matW;
	gl_Position = matWVP * vec4(vPosition.xyz, 1);
	normal = normalize(vNormal) * mat3(matW);

	vec3 rayDir = refract(camDir, normal, airToWater);

	vec3 bottom = rayDir * waterDepth / rayDir.z;

	position = vPosition.xz;
	texcoord = (vPosition.xz + bottom.xy) * vec2(0.5, -0.5) + vec2(0.5, 0.5);

	float mask = dot(normal, vec3(0, 0, 1));
	color = vec4(1, 1, 1, 1.0 - mask);
}