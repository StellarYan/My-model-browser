#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

uniform mat4 M2W;
uniform mat4 W2V;
uniform mat4 V2P;

out vec3 pos;
out vec3 nor;

void main()
{
	mat3 normalMat = transpose(inverse(mat3(M2W)));
	nor = normalMat *normal;
	pos = (M2W*vec4(position,1)).xyz;
    gl_Position =  V2P*W2V*M2W*vec4(position.x, position.y, position.z, 1.0);
}
