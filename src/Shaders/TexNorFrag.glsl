#version 330 core

in vec2 TexCoord;
in vec3 FragPosition;
in vec3 FNormal;
in mat3 normalMat;
in mat3 TBN;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform vec3 viewPos;

const float Attconstant = 1.0f;
const float Attlinear = 0.002f;
const float Attquadratic = 0.001f;


struct PointLight {
	vec3 position;
	vec3 color;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct DirLight {
	vec3 dirction;
	vec3 color;
};
#define NR_DIR_LIGHTS 1
uniform DirLight dirLights[NR_DIR_LIGHTS];

struct Material {
	float shiness;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
float near = 0.1f;
float far = 100.0;

vec3 CaculatePointLight(PointLight light, vec3 view, vec3 nor, vec3 pos, Material mat)
{
	vec3 L = normalize(light.position - pos);
	vec3 V = normalize(view - pos);
	vec3 H = (L + V) / length(L + V);

	
	vec3 N = normalize(nor);
	float distance = length(light.position - pos);
	float attenuation = 1.0f / (Attconstant + Attlinear * distance + Attquadratic * (distance * distance) );
	vec3 diff = attenuation * mat.diffuse * max(dot(L, N), 0) * light.color;
	vec3 spec = attenuation * mat.specular * pow(max((dot(N, H)), 0), mat.shiness) * light.color;

	return diff+spec;
}

vec3 CaculateDirLight(vec3 dirL,vec3 colorL,float Intensity,vec3 view, vec3 nor, vec3 pos, Material mat)
{
	
	
	vec3 L = normalize(dirL);
	vec3 V = normalize(view - pos);
	vec3 H = (L + V) / length(L + V);

	vec3 N = normalize(nor);
	float attenuation = 1.0f / Attconstant;
	vec3 diff = attenuation * mat.diffuse * max(dot(L, N), 0) *Intensity* colorL;
	vec3 spec = attenuation * mat.specular * pow(max((dot(N, H)), 0), mat.shiness)*Intensity * colorL;
	return diff+spec ;
}


void main()
{
	Material material;
	material.diffuse = texture(texture_diffuse1, TexCoord).rgb;
	material.specular = texture(texture_specular1, TexCoord).rgb;
	material.shiness = 10;
	material.ambient = vec3(0.1, 0.1, 0.1);

	vec3 tagNor = normalize(TBN* (normalize(texture(texture_normal1, TexCoord).rgb * 2.0 - 1.0)) );

	color = vec4(0, 0, 0, 0);
	for (int i = 0; i<NR_POINT_LIGHTS; i++)
	{
		vec3 Lrgb = CaculatePointLight(pointLights[i], viewPos, tagNor, FragPosition, material);
		color = color + vec4(Lrgb.r, Lrgb.g, Lrgb.b, 0);

	}
	vec3 dirL = vec3(0, 0, -1);
	vec3 ColorL = vec3(1, 1, 1);
	color += vec4(material.ambient, 0.0);

}
