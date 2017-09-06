#pragma once
#include "Lib.h"

enum ShaderType { ObjectShader, postEffectShader,WhiteShader,SkyboxShader };
struct MyShader
{
	GLShaderObject CompileVert(const std::string vert)
	{
		const GLchar* VertexShaderSrc[1] = { vert.data() };
		GLShaderObject vertexShader; //vertex shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER); //创建shader
		glShaderSource(vertexShader, 1, VertexShaderSrc, NULL); //指定shader文本
		glCompileShader(vertexShader); //编译shader

		GLchar  infoLog[512];
		GLint success;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		return vertexShader;
	}
	GLShaderObject CompileGeo(const std::string geo)
	{
		const GLchar* GeometryShaderSrc[1] = { geo.data() };
		GLShaderObject GeometryShader; //vertex shader
		GeometryShader = glCreateShader(GL_VERTEX_SHADER); //创建shader
		glShaderSource(GeometryShader, 1, GeometryShaderSrc, NULL); //指定shader文本
		glCompileShader(GeometryShader); //编译shader

		GLchar  infoLog[512];
		GLint success;
		glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(GeometryShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		return GeometryShader;
	}
	GLShaderObject CompileFrag(const std::string frag)
	{
		const GLchar* FragmentShaderSrc[1] = { frag.data() };
		GLShaderObject FragmentShader; // 同上
		FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(FragmentShader, 1, FragmentShaderSrc, NULL);
		glCompileShader(FragmentShader);

		GLchar  infoLog[512];
		GLint success;
		glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		return FragmentShader;
	}

public:
	GLShaderObject GLshader;
	std::string name;
	ShaderType type;
	MyShader()
	{

	}
	MyShader(const std::string vertPath, const std::string fragPath,ShaderType type):type(type)
	{
		std::string vertexShaderString = file2string(vertPath);
		std::string fragmentShaderString = file2string(fragPath);

		GLShaderObject vertShader = CompileVert(vertexShaderString);
		GLShaderObject fragShader = CompileFrag(fragmentShaderString);

		GLshader = glCreateProgram();
		glAttachShader(GLshader, vertShader);
		glAttachShader(GLshader, fragShader);
		glLinkProgram(GLshader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

	}
	MyShader(const std::string vertPath, const std::string geoPath, const std::string fragPath, ShaderType type):type(type)
	{
		std::string vertexShaderString = file2string(vertPath);
		std::string geometryShaderString = file2string(geoPath);
		std::string fragmentShaderString = file2string(fragPath);
		

		GLShaderObject vertShader = CompileVert(vertexShaderString);
		GLShaderObject geoShader = CompileGeo(geometryShaderString);
		GLShaderObject fragShader = CompileFrag(fragmentShaderString);
		GLshader = glCreateProgram();
		glAttachShader(GLshader, vertShader);
		glAttachShader(GLshader, geoShader);
		glAttachShader(GLshader, fragShader);
		glLinkProgram(GLshader);

		glDeleteShader(vertShader); //连接后就可以删除了
		glDeleteShader(geoShader);
		glDeleteShader(fragShader);
	}
	~MyShader()
	{
		glDeleteShader(GLshader);
	}

};
using pMyShader = std::shared_ptr<MyShader>;


struct Texture
{
public:
	GLTextureObject TextureID;
	aiTextureType type;
	std::string path;
	std::string name;
	GLuint height, width;
	//Texture() {}
	Texture(aiTextureType type,std::string path,std::string name):type(type),path(path),name(name)
	{
		FIBITMAP* tex = FreeImage_Load(FreeImage_GetFileType(path.data()), path.data(), PNG_IGNOREGAMMA);
		if (!tex) std::cout << "贴图载入失败" << std::endl;

		glGenTextures(1, &TextureID);

		height = FreeImage_GetWidth(tex);
		width = FreeImage_GetHeight(tex);

		FREE_IMAGE_COLOR_TYPE t = FreeImage_GetColorType(tex);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		GLenum imageFormat = GL_BGRA;
		GLenum ImageType = GL_UNSIGNED_BYTE;
		auto bpp = FreeImage_GetBPP(tex);
		if (bpp == 24)
		{
			if (type == aiTextureType::aiTextureType_NORMALS)
				imageFormat = GL_RGB; 
			else imageFormat = GL_BGR;
		}
		else if (bpp == 32)
		{
			imageFormat = GL_BGRA;
		}
		else if (bpp == 8)
		{
			imageFormat = GL_DEPTH_COMPONENT;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			height, width,
			0, imageFormat, ImageType, FreeImage_GetBits(tex));
		glGenerateMipmap(GL_TEXTURE_2D);
		FreeImage_Unload(tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	~Texture()
	{
		glDeleteTextures(1, &TextureID);
	}
};
using pTexture = std::shared_ptr<Texture>;

const std::vector<aiTextureType> Supportedtype = { aiTextureType_DIFFUSE,aiTextureType_SPECULAR,aiTextureType_HEIGHT};
class Material
{
public:
	pMyShader shader;
	static std::vector<pTexture> TextureArray; //纹理的静态容器
	std::vector<int> texIndices; //标记当前材质的纹理索引
	pTexture tex;
	std::string name;
	Material(const aiMaterial * mat,const std::string directory, const pMyShader shader,const std::string name):shader(shader),name(name)
	{
		for (int i = 0; i < Supportedtype.size(); i++)
		{
			std::vector<pTexture> Typetextures;
			for (GLuint j = 0; j < mat->GetTextureCount(Supportedtype[i]); j++)
			{
				aiString str;
				mat->GetTexture(Supportedtype[i], j, &str);
				
				std::string path = directory.data() + std::string("\\") + std::string(str.data);
				for (int x = 0; x < TextureArray.size(); x++)
				{
					if (TextureArray[x]->path == path)
					{
						
						texIndices.push_back(x);
						goto TEXBREAK;
					}
				}
				Typetextures.push_back(pTexture(new Texture(Supportedtype[i], path, str.data) ) );
				this->tex = Typetextures[Typetextures.size() - 1];

				texIndices.push_back(TextureArray.size()); // 指定当前材质的纹理
			}
			TEXBREAK:
			TextureArray.insert(TextureArray.end(), Typetextures.begin(), Typetextures.end());
		}
		
	}
	void Use() const
	{
		if (shader->type == ShaderType::ObjectShader)
		{
			GLuint diffuseNr = 1;
			GLuint specularNr = 1;
			GLuint normalNr = 1;
			glUseProgram(shader->GLshader);
			for (GLuint i = 0; i < this->texIndices.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + texIndices[i]);
				std::string name;
				switch (this->TextureArray[texIndices[i]]->type)
				{
				case aiTextureType::aiTextureType_DIFFUSE:
					name = "texture_diffuse" + std::to_string(diffuseNr++);
					break;
				case aiTextureType::aiTextureType_SPECULAR:
					name = "texture_specular" + std::to_string(specularNr++);
					break;
				case aiTextureType::aiTextureType_HEIGHT:
					name = "texture_normal" + std::to_string(normalNr++);
					break;
				default:
					break;
				}
				GLLocation loc = glGetUniformLocation(shader->GLshader, (name).c_str());
				glUniform1i(glGetUniformLocation(shader->GLshader, (name).c_str()), texIndices[i]);
				glBindTexture(GL_TEXTURE_2D, this->TextureArray[texIndices[i]]->TextureID);
			}
			glActiveTexture(GL_TEXTURE0);
		}
		else if (shader->type == ShaderType::WhiteShader)
		{
			glUseProgram(shader->GLshader);
		}

	}
};
using pMaterial = std::shared_ptr<Material>;


std::vector<pTexture> Material::TextureArray;