#version 330 core

in vec2 TexCoord;


uniform sampler2D tex;
out vec4 color;

const float offset = 1.0 / 300;

void main()
{

	//反色处理
	//color = vec4(vec3(1.0 - texture(tex, TexCoord)), 1.0);  

	//Kernel effects  滤波处理
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),  // top-left
		vec2(0.0f, offset),  // top-center
		vec2(offset, offset),  // top-right
		vec2(-offset, 0.0f),    // center-left
		vec2(0.0f, 0.0f),    // center-center
		vec2(offset, 0.0f),    // center-right
		vec2(-offset, -offset), // bottom-left
		vec2(0.0f, -offset), // bottom-center
		vec2(offset, -offset)  // bottom-right    
		);

	float kernel[9] = float[](
		-1, -2, -1,
		-2, 12, -2,
		-1, -2, -1
		);

	vec3 sampleTex[9];
	for (int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(tex, TexCoord.st + offsets[i]));
	}
	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++)
		col += sampleTex[i] * kernel[i];

	color = vec4(col, 1.0);
	
}
