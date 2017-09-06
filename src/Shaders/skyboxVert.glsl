#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 V2P;
uniform mat4 W2V;


void main()
{
  vec4 pos = V2P * W2V * vec4(position, 1.0);
  gl_Position = pos.xyww;
  TexCoords = position;
  //TexCoords.y = -position.y;
  //TexCoords.x = -position.x;
  //TexCoords.y = -TexCoords.y;
}
