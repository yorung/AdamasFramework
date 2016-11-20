precision mediump float;
attribute vec3 vPosition;
attribute vec3 vNormal;
varying vec2 texcoord;
varying vec2 position;
varying vec3 normal;
varying vec4 color;
uniform vec4 b6[13];

const float airToWater = 1.0 / 1.33333;
const vec3 camDir = vec3(0, 0, -1);
const float waterDepth = 0.2;

void main()
{
	mat4 matW = mat4(b6[0], b6[1], b6[2], b6[3]);
	mat4 matV = mat4(b6[4], b6[5], b6[6], b6[7]);
	mat4 matP = mat4(b6[8], b6[9], b6[10], b6[11]);
	float time = b6[12].x;

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
