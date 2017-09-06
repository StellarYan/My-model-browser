#version 330 core

layout (location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec2 TexCoord;




uniform mat4 M2W;
uniform mat4 W2V;
uniform mat4 V2P;




void main()
{
    gl_Position =  V2P*W2V*M2W*vec4(position.x, position.y, position.z, 1.0);
	TexCoord = texcoord;
}
