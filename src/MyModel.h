#pragma once
#include "Lib.h"
#include "MyMaterial.h"

#define TOLE 1e-06

float degree2Rad(float degree)
{
	return degree * (PI / 180.0f);
}

float Rad2degree(float radians)
{
	return radians *(180.f / PI);
}


enum SpaceMode
{
	local,world
};

using Quaternion =glm::fquat;
struct Transform
{
	glm::vec3 position;
	glm::vec3 scale; 
	Quaternion quaternion;
};

//存在于空间中的所有物体的基类
class TransObject
{
protected:
	//glm::mat4 transform; 这里的tranform是local的
	Transform transform;
	std::vector<TransObject> childs;
public:
	TransObject()
	{
		transform.position = glm::vec3(0, 0, 0);
		transform.scale = glm::vec3(1, 1, 1);
		transform.quaternion = Quaternion();
	}
	void AddTransMatrix(glm::mat4 trans)
	{
		
		//https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
		transform.position = transform.position + glm::vec3(trans[3]);
		float s1 = glm::length(trans[0]);
		float s2 = glm::length(trans[1]);
		float s3 = glm::length(trans[2]);
		transform.scale = transform.scale*glm::vec3(s1, s2, s3);
		trans[0] = trans[0] / s1;
		trans[1] = trans[1] / s2;
		trans[2] = trans[2] / s3;
		transform.quaternion= glm::quat_cast(trans)*transform.quaternion;
	}
	void SetTransMatrix(glm::mat4 trans)
	{
		transform.position = glm::vec3(trans[3]);
		float s1 = glm::length(trans[0]);
		float s2 = glm::length(trans[1]);
		float s3 = glm::length(trans[2]);
		transform.scale = glm::vec3(s1, s2, s3);
		trans[0] = trans[0] / s1;
		trans[1] = trans[1] / s2;
		trans[2] = trans[2] / s3;
		transform.quaternion = glm::quat_cast(trans);
		
	}

	void Move(const glm::vec3& v)
	{
		transform.position += v;
		for (TransObject& t : childs) t.Move(v);
	}

	void Rotate(const glm::vec3& axis, float degree)
	{
		transform.quaternion = glm::rotate(transform.quaternion, degree2Rad(degree), axis);
		for (TransObject& t : childs) t.Rotate(axis, degree);
	}

	void Rotate(const Quaternion& quaternion)
	{
		transform.quaternion = quaternion*transform.quaternion;
		for (TransObject& t : childs) t.Rotate(quaternion);
	}

	void Scale(const glm::vec3& v)
	{
		transform.scale = transform.scale*v;
		for (TransObject& t : childs) t.Scale(v);
	}

	void RotateTo(const glm::vec3& direction,const glm::vec3& up)
	{
		Quaternion target;
		glm::mat3 l = glm::inverse( glm::lookAt(transform.position, transform.position + direction, up));
		target = glm::quat_cast(l);
 		RotateTo(target);
	}
	void RotateTo(const Quaternion& quaternion)
	{
		Rotate(quaternion*glm::inverse(transform.quaternion));
	}
	//变换的顺序是缩放，旋转，平移，即transform矩阵为T*R*S
	glm::mat4 TRS() const 
	{ 
		glm::mat4 R = mat4_cast(transform.quaternion);
		glm::mat4 S = glm::scale(glm::mat4(), transform.scale);
		glm::mat4 T = glm::translate(glm::mat4(), transform.position);
		
		return T*R*S;
	}
	glm::mat4 M2W() const { return this->TRS(); }
	//x-pitch y-yaw z-roll
	glm::vec3 EularAngles() const{return glm::eulerAngles(transform.quaternion);}
	glm::vec3 Forward()const { return glm::normalize(transform.quaternion*glm::vec3(0, 0, 1)); }
	glm::vec3 Up() const{return glm::normalize(transform.quaternion*glm::vec3(0, 1, 0));}
	glm::vec3 Left() const{return glm::normalize(transform.quaternion*glm::vec3(1, 0, 0));}
	const glm::vec3& GetPosition() const{return transform.position;}
	const glm::vec3& GetScale() const{return transform.scale;}
	const Quaternion& GetRotation() const { return transform.quaternion; }

	void SetPosition(glm::vec3 position) {Move(position - transform.position); }
	void SetScale(glm::vec3 scale) { Scale(scale / transform.scale); }
	//旋转时的顺序是pitch yaw roll，输入的是角度值
	void SetEular(glm::vec3 eularAngle) 
	{
		Quaternion target;
		target = glm::rotate(target, degree2Rad(eularAngle.x), glm::vec3(1, 0, 0));
		target = glm::rotate(target, degree2Rad(eularAngle.y), glm::vec3(0, 1, 0));
		target = glm::rotate(target, degree2Rad(eularAngle.z), glm::vec3(0, 0, 1));
		RotateTo(target);
	}
};


struct Vertex
{
	glm::vec3 position;
	glm::vec3 uvw;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	
	Vertex(const glm::vec3 position) :position(position) {};
	Vertex(const glm::vec3 position, const glm::vec3 normal, const glm::vec2 uv) :position(position), normal(normal), uvw(uvw) {};
	Vertex(const glm::vec3 position, const glm::vec3 normal, const glm::vec3 uvw, const glm::vec3 tangent, const glm::vec3 bitangent):
	position(position), normal(normal), uvw(uvw), tangent(tangent), bitangent(bitangent) {};

	
};

class Mesh
{
public:
	pMaterial mat;
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	GLBufferObject VAO,VBO,EBO;

	void SetMaterial(pMaterial mat)
	{
		this->mat = mat;
	}
	Mesh(std::vector<Vertex> vertices, std::vector<int> indices):vertices(vertices),indices(indices)
	{
		
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
			&this->vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
			&this->indices[0], GL_STATIC_DRAW);


		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, Vertex::position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, Vertex::uvw));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, Vertex::normal));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, Vertex::tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, Vertex::bitangent));
		glBindVertexArray(0);

	}
	void Draw(glm::mat4 M2W,glm::mat4 W2V,glm::mat4 V2P) const
	{
		mat->Use();
		GLShaderObject shader = mat->shader->GLshader;
		glUniformMatrix4fv(glGetUniformLocation(shader, "M2W"), 1, GL_FALSE, glm::value_ptr(M2W));
		glUniformMatrix4fv(glGetUniformLocation(shader, "W2V"), 1, GL_FALSE, glm::value_ptr(W2V));
		glUniformMatrix4fv(glGetUniformLocation(shader, "V2P"), 1, GL_FALSE, glm::value_ptr(V2P));
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);

	}
	~Mesh()
	{
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
};
using pMesh = std::shared_ptr<Mesh>;

class Model : public TransObject
{
public:
	std::string path;
	std::vector<pMesh> meshes;
	glm::mat4 OBB;  //OBB处于物体空间中
	glm::vec3 center;
	float DX, DY, DZ;


	void Draw(glm::mat4 W2V,glm::mat4 V2P) const
	{
		for (int i=0;i<meshes.size();i++)
		{
			meshes[i]->Draw(M2W(),W2V,V2P);
		}
	}

	//Oriented bounding box https://en.wikipedia.org/wiki/Minimum_bounding_box
	void SetupOBB()
	{
		const Vertex* f = &meshes[0]->vertices[0];
		const Vertex* maxX=f, *minX=f, *maxY=f, *minY=f, *maxZ=f, *minZ=f;
		
		for (int i = 0; i < meshes.size(); i++)
		{
			for (int j = 0; j < meshes[i]->indices.size(); j++)
			{
				const Vertex* v = &(meshes[i]->vertices[meshes[i]->indices[j]]);
				maxX = v->position.x > maxX->position.x ? v : maxX;
				minX = v->position.x < minX->position.x ? v : minX;
				maxY = v->position.y > maxY->position.y ? v : maxY;
				minY = v->position.y < minY->position.y ? v : minY;
				maxZ = v->position.z > maxZ->position.z ? v : maxZ;
				minZ = v->position.z < minZ->position.z ? v : minZ;
			}
		}
		DX = (maxX->position.x - minX->position.x);
		DY = (maxY->position.y - minY->position.y);
		DZ = (maxZ->position.z - minZ->position.z);
		center = 0.5f*glm::vec3(maxX->position.x + minX->position.x,maxY->position.y + minY->position.y, maxZ->position.z + minZ->position.z);
		OBB = glm::translate(glm::mat4(), center);
		OBB = glm::scale(OBB, glm::vec3(DX, DY, DZ));
	}
	Model() {}
};
using pModel = std::shared_ptr<Model>;


struct SimpleObject : public TransObject
{
	GLBufferObject vao;
	GLBufferObject vbo;
	GLBufferObject ebo;
	GLTextureObject texture;
	pMyShader shader;
	GLenum DrawMode;
	int DrawCount;


	void Draw(glm::mat4 W2V,glm::mat4 V2P)
	{
		glUseProgram(shader->GLshader);
		glUniformMatrix4fv(glGetUniformLocation(shader->GLshader, "M2W"), 1, GL_FALSE, glm::value_ptr(TRS()));
		glUniformMatrix4fv(glGetUniformLocation(shader->GLshader, "W2V"), 1, GL_FALSE, glm::value_ptr(W2V));
		glUniformMatrix4fv(glGetUniformLocation(shader->GLshader, "V2P"), 1, GL_FALSE, glm::value_ptr(V2P));
		if (texture != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(shader->GLshader, "tex"), 0);
			glBindTexture(GL_TEXTURE_2D, texture);
		}
		glBindVertexArray(vao);
		if(ebo ==0)
			glDrawArrays(DrawMode, 0, DrawCount);
		else 
			glDrawElements(DrawMode, DrawCount, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);

	}

	//注意这里顶点只有uv坐标，没有w坐标
	SimpleObject(GLfloat* xyzuv_Vert, GLfloat VertCount , GLTextureObject texture, pMyShader shader,GLenum DrawMode) :DrawCount(VertCount),DrawMode(DrawMode), shader(shader),texture(texture)
	{
		ebo = 0;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, VertCount * 5 * sizeof(GLfloat), xyzuv_Vert, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	SimpleObject(GLfloat* xyz_Vert, GLfloat VertCount, pMyShader shader, GLenum DrawMode):DrawCount(VertCount),shader(shader),DrawMode(DrawMode),texture(0)
	{
		ebo = 0;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, VertCount * 3 * sizeof(GLfloat), xyz_Vert, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
				glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void AddEBO(GLuint* indices,int count)
	{
		if (ebo == 0)
		{
			glGenBuffers(1, &ebo);
			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count* sizeof(GLfloat),
				indices, GL_STATIC_DRAW);


			glEnableVertexAttribArray(0);
		}
	}

	SimpleObject() {}
	~SimpleObject()
	{
		std::cout << "AAAAAAAAAA" << std::endl;
		
		glDeleteBuffers(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteTextures(1, &texture);
		
	
	}
};
using pSimpleObject = std::shared_ptr<SimpleObject>;

GLfloat LineVertices[] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f
};

GLfloat Cubevertices[] = {
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f
};


GLfloat SphereVertices[] = 
{
	   0.0000f,  1.0000f,- 0.0000f,
	 - 0.0000f,  0.9239f,- 0.3827f,
	 - 0.1464f,  0.9239f,- 0.3536f,
	 - 0.2706f,  0.9239f,- 0.2706f,
	 - 0.3536f,  0.9239f,- 0.1464f,
	 - 0.3827f,  0.9239f,- 0.0000f,
	 - 0.3536f,  0.9239f,  0.1464f,
	 - 0.2706f,  0.9239f,  0.2706f,
	 - 0.1464f,  0.9239f,  0.3536f,
	   0.0000f,  0.9239f,  0.3827f,
	   0.1464f,  0.9239f,  0.3536f,
	   0.2706f,  0.9239f,  0.2706f,
	   0.3536f,  0.9239f,  0.1464f,
	   0.3827f,  0.9239f,- 0.0000f,
	   0.3536f,  0.9239f,- 0.1464f,
	   0.2706f,  0.9239f,- 0.2706f,
	   0.1464f,  0.9239f,- 0.3536f,
	 - 0.0000f,  0.7071f,- 0.7071f,
	 - 0.2706f,  0.7071f,- 0.6533f,
	 - 0.5000f,  0.7071f,- 0.5000f,
	 - 0.6533f,  0.7071f,- 0.2706f,
	 - 0.7071f,  0.7071f,- 0.0000f,
	 - 0.6533f,  0.7071f,  0.2706f,
	 - 0.5000f,  0.7071f,  0.5000f,
	 - 0.2706f,  0.7071f,  0.6533f,
	   0.0000f,  0.7071f,  0.7071f,
	   0.2706f,  0.7071f,  0.6533f,
	   0.5000f,  0.7071f,  0.5000f,
	   0.6533f,  0.7071f,  0.2706f,
	   0.7071f,  0.7071f,- 0.0000f,
	   0.6533f,  0.7071f,- 0.2706f,
	   0.5000f,  0.7071f,- 0.5000f,
	   0.2706f,  0.7071f,- 0.6533f,
	 - 0.0000f,  0.3827f,- 0.9239f,
	 - 0.3536f,  0.3827f,- 0.8536f,
	 - 0.6533f,  0.3827f,- 0.6533f,
	 - 0.8536f,  0.3827f,- 0.3536f,
	 - 0.9239f,  0.3827f,- 0.0000f,
	 - 0.8536f,  0.3827f,  0.3536f,
	 - 0.6533f,  0.3827f,  0.6533f,
	 - 0.3536f,  0.3827f,  0.8536f,
	   0.0000f,  0.3827f,  0.9239f,
	   0.3536f,  0.3827f,  0.8536f,
	   0.6533f,  0.3827f,  0.6533f,
	   0.8536f,  0.3827f,  0.3536f,
	   0.9239f,  0.3827f,- 0.0000f,
	   0.8536f,  0.3827f,- 0.3536f,
	   0.6533f,  0.3827f,- 0.6533f,
	   0.3536f,  0.3827f,- 0.8536f,
	 - 0.0000f,- 0.0000f,- 1.0000f,
	 - 0.3827f,- 0.0000f,- 0.9239f,
	 - 0.7071f,- 0.0000f,- 0.7071f,
	 - 0.9239f,- 0.0000f,- 0.3827f,
	 - 1.0000f,- 0.0000f,- 0.0000f,
	 - 0.9239f,- 0.0000f,  0.3827f,
	 - 0.7071f,- 0.0000f,  0.7071f,
	 - 0.3827f,- 0.0000f,  0.9239f,
	   0.0000f,- 0.0000f,  1.0000f,
	   0.3827f,- 0.0000f,  0.9239f,
	   0.7071f,- 0.0000f,  0.7071f,
	   0.9239f,- 0.0000f,  0.3827f,
	   1.0000f,- 0.0000f,- 0.0000f,
	   0.9239f,- 0.0000f,- 0.3827f,
	   0.7071f,- 0.0000f,- 0.7071f,
	   0.3827f,- 0.0000f,- 0.9239f,
	 - 0.0000f,- 0.3827f,- 0.9239f,
	 - 0.3536f,- 0.3827f,- 0.8536f,
	 - 0.6533f,- 0.3827f,- 0.6533f,
	 - 0.8536f,- 0.3827f,- 0.3536f,
	 - 0.9239f,- 0.3827f,- 0.0000f,
	 - 0.8536f,- 0.3827f,  0.3536f,
	 - 0.6533f,- 0.3827f,  0.6533f,
	 - 0.3536f,- 0.3827f,  0.8536f,
	   0.0000f,- 0.3827f,  0.9239f,
	   0.3536f,- 0.3827f,  0.8536f,
	   0.6533f,- 0.3827f,  0.6533f,
	   0.8536f,- 0.3827f,  0.3536f,
	   0.9239f,- 0.3827f,- 0.0000f,
	   0.8536f,- 0.3827f,- 0.3536f,
	   0.6533f,- 0.3827f,- 0.6533f,
	   0.3536f,- 0.3827f,- 0.8536f,
	 - 0.0000f,- 0.7071f,- 0.7071f,
	 - 0.2706f,- 0.7071f,- 0.6533f,
	 - 0.5000f,- 0.7071f,- 0.5000f,
	 - 0.6533f,- 0.7071f,- 0.2706f,
	 - 0.7071f,- 0.7071f,- 0.0000f,
	 - 0.6533f,- 0.7071f,  0.2706f,
	 - 0.5000f,- 0.7071f,  0.5000f,
	 - 0.2706f,- 0.7071f,  0.6533f,
	   0.0000f,- 0.7071f,  0.7071f,
	   0.2706f,- 0.7071f,  0.6533f,
	   0.5000f,- 0.7071f,  0.5000f,
	   0.6533f,- 0.7071f,  0.2706f,
	   0.7071f,- 0.7071f,- 0.0000f,
	   0.6533f,- 0.7071f,- 0.2706f,
	   0.5000f,- 0.7071f,- 0.5000f,
	   0.2706f,- 0.7071f,- 0.6533f,
	 - 0.0000f,- 0.9239f,- 0.3827f,
	 - 0.1464f,- 0.9239f,- 0.3536f,
	 - 0.2706f,- 0.9239f,- 0.2706f,
	 - 0.3536f,- 0.9239f,- 0.1464f,
	 - 0.3827f,- 0.9239f,- 0.0000f,
	 - 0.3536f,- 0.9239f,  0.1464f,
	 - 0.2706f,- 0.9239f,  0.2706f,
	 - 0.1464f,- 0.9239f,  0.3536f,
	   0.0000f,- 0.9239f,  0.3827f,
	   0.1464f,- 0.9239f,  0.3536f,
	   0.2706f,- 0.9239f,  0.2706f,
	   0.3536f,- 0.9239f,  0.1464f,
	   0.3827f,- 0.9239f,- 0.0000f,
	   0.3536f,- 0.9239f,- 0.1464f,
	   0.2706f,- 0.9239f,- 0.2706f,
	   0.1464f,- 0.9239f,- 0.3536f,
	   0.0000f,- 1.0000f,- 0.0000f



};



GLuint SphereIndices[]=
{
	1 , 2   ,3    ,
	1 ,	3   ,4 	  ,
	1 ,	4   ,5 	  ,
	1 ,	5   ,6 	  ,
	1 ,	6   ,7 	  ,
	1 ,	7   ,8 	  ,
	1 ,	8   ,9 	  ,
	1 ,	9   ,10	  ,
	1 ,	10	,11	  ,
	1 ,	11	,12	  ,
	1 ,	12	,13	  ,
	1 ,	13	,14	  ,
	1 ,	14	,15	  ,
	1 ,	15	,16	  ,
	1 ,	16	,17	  ,
	1 ,	17	,2 	  ,
	2 ,	18	,19	  ,
	2 ,	19	,3 	  ,
	3 ,	19	,20	  ,
	3 ,	20	,4 	  ,
	4 ,	20	,21	  ,
	4 ,	21	,5 	  ,
	5 ,	21	,22	  ,
	5 ,	22	,6 	  ,
	6 ,	22	,23	  ,
	6 ,	23	,7 	  ,
	7 ,	23	,24	  ,
	7 ,	24	,8 	  ,
	8 ,	24	,25	  ,
	8 ,	25	,9 	  ,
	9 ,	25	,26	  ,
	9 ,	26	,10	  ,
	10,	26	,27	  ,
	10,	27	,11	  ,
	11,	27	,28	  ,
	11,	28	,12	  ,
	12,	28	,29	  ,
	12,	29	,13	  ,
	13,	29	,30	  ,
	13,	30	,14	  ,
	14,	30	,31	  ,
	14,	31	,15	  ,
	15,	31	,32	  ,
	15,	32	,16	  ,
	16,	32	,33	  ,
	16,	33	,17	  ,
	17,	33	,18	  ,
	17,	18	,2 	  ,
	18,	34	,35	  ,
	18,	35	,19	  ,
	19,	35	,36	  ,
	19,	36	,20	  ,
	20,	36	,37	  ,
	20,	37	,21	  ,
	21,	37	,38	  ,
	21,	38	,22	  ,
	22,	38	,39	  ,
	22,	39	,23	  ,
	23,	39	,40	  ,
	23,	40	,24	  ,
	24,	40	,41	  ,
	24,	41	,25	  ,
	25,	41	,42	  ,
	25,	42	,26	  ,
	26,	42	,43	  ,
	26,	43	,27	  ,
	27,	43	,44	  ,
	27,	44	,28	  ,
	28,	44	,45	  ,
	28,	45	,29	  ,
	29,	45	,46	  ,
	29,	46	,30	  ,
	30,	46	,47	  ,
	30,	47	,31	  ,
	31,	47	,48	  ,
	31,	48	,32	  ,
	32,	48	,49	  ,
	32,	49	,33	  ,
	33,	49	,34	  ,
	33,	34	,18	  ,
	34,	50	,51	  ,
	34,	51	,35	  ,
	35,	51	,52	  ,
	35,	52	,36	  ,
	36,	52	,53	  ,
	36,	53	,37	  ,
	37,	53	,54	  ,
	37,	54	,38	  ,
	38,	54	,55	  ,
	38,	55	,39	  ,
	39,	55	,56	  ,
	39,	56	,40	  ,
	40,	56	,57	  ,
	40,	57	,41	  ,
	41,	57	,58	  ,
	41,	58	,42	  ,
	42,	58	,59	  ,
	42,	59	,43	  ,
	43,	59	,60	  ,
	43,	60	,44	  ,
	44,	60	,61	  ,
	44,	61	,45	  ,
	45,	61	,62	  ,
	45,	62	,46	  ,
	46,	62	,63	  ,
	46,	63	,47	  ,
	47,	63	,64	  ,
	47,	64	,48	  ,
	48,	64	,65	  ,
	48,	65	,49	  ,
	49,	65	,50	  ,
	49,	50	,34	  ,
	50,	66	,67	  ,
	50,	67	,51	  ,
	51,	67	,68	  ,
	51,	68	,52	  ,
	52,	68	,69	  ,
	52,	69	,53	  ,
	53,	69	,70	  ,
	53,	70	,54	  ,
	54,	70	,71	  ,
	54,	71	,55	  ,
	55,	71	,72	  ,
	55,	72	,56	  ,
	56,	72	,73	  ,
	56,	73	,57	  ,
	57,	73	,74	  ,
	57,	74	,58	  ,
	58,	74	,75	  ,
	58,	75	,59	  ,
	59,	75	,76	  ,
	59,	76	,60	  ,
	60,	76	,77	  ,
	60,	77	,61	  ,
	61,	77	,78	  ,
	61,	78	,62	  ,
	62,	78	,79	  ,
	62,	79	,63	  ,
	63,	79	,80	  ,
	63,	80	,64	  ,
	64,	80	,81	  ,
	64,	81	,65	  ,
	65,	81	,66	  ,
	65,	66	,50	  ,
	66,	82	,83	  ,
	66,	83	,67	  ,
	67,	83	,84	  ,
	67,	84	,68	  ,
	68,	84	,85	  ,
	68,	85	,69	  ,
	69,	85	,86	  ,
	69,	86	,70	  ,
	70,	86	,87	  ,
	70,	87	,71	  ,
	71,	87	,88	  ,
	71,	88	,72	  ,
	72,	88	,89	  ,
	72,	89	,73	  ,
	73,	89	,90	  ,
	73,	90	,74	  ,
	74,	90	,91	  ,
	74,	91	,75	  ,
	75,	91	,92	  ,
	75,	92	,76	  ,
	76,	92	,93	  ,
	76,	93	,77	  ,
	77,	93	,94	  ,
	77,	94	,78	  ,
	78,	94	,95	  ,
	78,	95	,79	  ,
	79,	95	,96	  ,
	79,	96	,80	  ,
	80,	96	,97	  ,
	80,	97	,81	  ,
	81,	97	,82	  ,
	81,	82	,66	  ,
	82,	98	,99	  ,
	82,	99	,83	  ,
	83,	99	,100  ,
	83,	100	,84	  ,
	84,	100	,101  ,
	84,	101	,85	  ,
	85,	101	,102  ,
	85,	102	,86	  ,
	86,	102	,103  ,
	86,	103	,87	  ,
	87,	103	,104  ,
	87,	104	,88	  ,
	88,	104	,105  ,
	88,	105	,89	  ,
	89,	105	,106  ,
	89,	106	,90	  ,
	90,	106	,107  ,
	90,	107	,91	  ,
	91,	107	,108  ,
	91,	108	,92	  ,
	92,	108	,109  ,
	92,	109	,93	  ,
	93,	109	,110  ,
	93,	110	,94	  ,
	94,	110	,111  ,
	94,	111	,95	  ,
	95,	111	,112  ,
	95,	112	,96	  ,
	96,	112	,113  ,
	96,	113	,97	  ,
	97,	113	,98	  ,
	97,	98	,82	  ,
	114,99	,98	  ,
	114,100	,99	  ,
	114,101	,100  ,
	114,102	,101  ,
	114,103	,102  ,
	114,104	,103  ,
	114,105	,104  ,
	114,106	,105  ,
	114,107	,106  ,
	114,108	,107  ,
	114,109	,108  ,
	114,110	,109  ,
	114,111	,110  ,
	114,112	,111  ,
	114,113	,112  ,
	114,98	,113  

};

