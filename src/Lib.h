#pragma once
_CRT_SECURE_NO_WARNINGS


#define GLEW_STATIC
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include <GL\glew.h>
#include <GLFW\glfw3.h>


#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <glm\glm.hpp>
#include <glm\matrix.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>

#include <FreeImage.h>






#define PI 3.141592
using GLBufferObject = GLuint;
using GLShaderObject = GLuint;
using GLTextureObject = GLuint;
using GLLocation = GLuint;

std::string file2string(std::string address)
{
	std::ifstream f;

	f.open(address.data());
	if (!f.is_open()) return std::string();
	f.seekg(0, f.end);
	int length = f.tellg();

	std::string s;
	s.resize(length + 1);
	f.seekg(0, f.beg);
	f.read(&s[0], length);
	s.resize(f.gcount() + 1);
	s[f.gcount()] = '\0';
	return s;
}






