#version 330 core

out vec4 color;

in vec3 pos;
in vec3 nor;



struct PointLight {
	vec3 position;
	vec3 color;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

void main()
{
	PointLight light = pointLights[0];
	vec3 L = normalize(light.position - pos);
	vec3 N = nor;
	vec3 white = vec3(1, 1, 1);
	vec3 diff =  white * max(dot(L, N), 0) * light.color;
	color = vec4(diff,0);
}
