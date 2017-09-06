#pragma once

#include "Lib.h"
#include "MyMaterial.h"

GLfloat skyboxVertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

class Skybox
{
	GLTextureObject cubemap;
	pMyShader skyboxShader;

	static GLBufferObject vao;
	static GLBufferObject vbo;
	static bool isInitCubemap;
	static void initSkyBox()
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER,  sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
				glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		isInitCubemap = true;
	}



public:
	Skybox(std::string skyboxDirectory)
	{
		if(!isInitCubemap) initSkyBox();
		skyboxShader = pMyShader(new MyShader(".\\Shaders\\skyboxVert.glsl", ".\\Shaders\\skyboxFrag.glsl", ShaderType::SkyboxShader));
		std::string cubemapFacePath[6] = { std::string(skyboxDirectory + "\\left.tga"),std::string(skyboxDirectory + "\\right.tga")
			,std::string(skyboxDirectory + "\\top.tga"),std::string(skyboxDirectory + "\\botton.tga") ,
			std::string(skyboxDirectory + "\\front.tga") ,std::string(skyboxDirectory + "\\back.tga") };
		glGenTextures(1, &cubemap);
		
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
		for (GLuint i = 0; i < 6; i++)
		{
			FIBITMAP* tex = FreeImage_Load(FreeImage_GetFileType(cubemapFacePath[i].data()), cubemapFacePath[i].data(), PNG_IGNOREGAMMA);

			//cubemap's wrapmode
			//http://stackoverflow.com/questions/11685608/convention-of-faces-in-opengl-cubemapping#comment42259181_11694336

			if(i==2 ) tex = FreeImage_Rotate(tex,-90);
			if(i==3) tex = FreeImage_Rotate(tex, 90);
			FreeImage_FlipVertical(tex);

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				
				GL_RGB, FreeImage_GetWidth(tex), FreeImage_GetHeight(tex), 0, GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(tex)
			);
			FreeImage_Unload(tex);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	};
	void DrawSkybox(glm::mat4 W2V,glm::mat4 V2P )
	{
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyboxShader->GLshader);
		glm::mat4 skyboxW2V = glm::mat4(glm::mat3(W2V));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->GLshader, "W2V"), 1, GL_FALSE, glm::value_ptr(skyboxW2V));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->GLshader, "V2P"), 1, GL_FALSE, glm::value_ptr(V2P));
		glBindVertexArray(vao);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(skyboxShader->GLshader, "skybox"), 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glUseProgram(skyboxShader->GLshader);

		glDepthFunc(GL_LESS);
		
	}

	~Skybox()
	{
		
		glDeleteTextures(1, &cubemap);
		glDeleteBuffers(1, &vao);
		glDeleteBuffers(1, &vbo);
		
	}
};
bool Skybox::isInitCubemap = false;
GLBufferObject Skybox::vao = 0;
GLBufferObject Skybox::vbo = 0;

using pSkybox = std::shared_ptr<Skybox>;
