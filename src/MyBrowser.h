#pragma once
#include "Lib.h"

#include "Controller.h"
#include "MyModel.h"
#include "Skybox.h"
#include "Ray.h"

using Window = std::shared_ptr<GLFWwindow >;
class MyCamera :public TransObject
{
public:
	float camNear;
	float camFar;
	glm::vec2 FOV;  //向量分量取0到PI/2
	MyCamera(glm::vec3 position=glm::vec3(0,0,0), float camNear = 0.1f, float camFar = 500.0f, glm::vec2 FOV = glm::vec2(45, 45))
	{
		this->transform.position = position;
		this->camNear = camNear;
		this->camFar = camFar;
		this->FOV = FOV;
	}
};
using pMyCamera = std::shared_ptr<MyCamera>;


class Browser
{
public:
	std::vector<pMyShader> shaderArray;
	std::vector<pModel> modelPool;
	std::vector<pMaterial> MaterialArray;
	

		
	pMyCamera camera;
	Window window;

	int window_height;
	int window_width;

	glm::mat4 mat_W2V;
	glm::mat4 mat_V2P;
	
private:
	pMyShader GetWhiteShader()
	{
		for (pMyShader s : shaderArray) if (s->type == ShaderType::WhiteShader)return s;
	}
	
	pMyShader GetPostEffectShader()
	{
		for (pMyShader s : shaderArray) if (s->type == ShaderType::postEffectShader )return s;
	}

	pModel SelectingModel;

	pSimpleObject standardCube;
	pSimpleObject standardSphere;

	GLBufferObject postEffect_fbo;
	GLBufferObject postEffect_rbo;
	pSimpleObject PostEffectCanvas;
	bool enablePostEffect;
	pSkybox skybox;
	
	void LoadShaders()
	{

		this->importShaderFile(
			".\\Shaders\\PostVertexShader.glsl", ".\\Shaders\\PostFragmentShader.glsl"
			, ShaderType::postEffectShader);
		this->importShaderFile(
			".\\Shaders\\TexNorVert.glsl", ".\\Shaders\\TexNorFrag.glsl"
			, ShaderType::ObjectShader);
		this->importShaderFile(
			".\\Shaders\\TexVert.glsl", ".\\Shaders\\TexFrag.glsl"
			, ShaderType::ObjectShader);
		this->importShaderFile(
			".\\Shaders\\WhiteVertShader.glsl", ".\\Shaders\\WhiteFragShader.glsl"
			, ShaderType::WhiteShader);


	}

	void LoadModel()
	{
		this->ImportModelFile(".\\tank\\tank.obj");
		this->ImportModelFile(".\\nano\\nanosuit.obj");
	}
	
public:
	void EnablePostEffect()
	{
		if (!enablePostEffect)
		{
			GLfloat quadVertices[] = {
				-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
				1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
				1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
				1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
				-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f,  0.0f, 0.0f
			};
			enablePostEffect = true;
			GLTextureObject postEffect_texture;
			glGenFramebuffers(1, &postEffect_fbo);
			glGenRenderbuffers(1, &postEffect_rbo);
			glGenTextures(1, &postEffect_texture);
			glBindFramebuffer(GL_FRAMEBUFFER, postEffect_fbo);
			
				glBindTexture(GL_TEXTURE_2D, postEffect_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postEffect_texture, 0);
				
				glBindRenderbuffer(GL_RENDERBUFFER, postEffect_rbo);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height); // Use a single renderbuffer object for both a depth AND stencil buffer.
				glBindRenderbuffer(GL_RENDERBUFFER, 0);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, postEffect_rbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			PostEffectCanvas = pSimpleObject(new SimpleObject(quadVertices, 6, postEffect_texture, GetPostEffectShader(), GL_TRIANGLES));
			std::cout << "后期处理效果以开启" << std::endl;
		}
	}

	void DisablePostEffect()
	{
		enablePostEffect = false;
	}

	Browser(std::string windowName, int width,int height):window_height(height), window_width(width)
	{

		glfwInit(); //glfw初始化
					//glfwWindowHint为下次glfwCreateWindow提供信息
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //主版本号
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //次版本号
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //严格要求版本号
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); //窗口是否可以被用户拉伸
											  										  
		this->window = Window(glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr), glfwDestroyWindow); //http://stackoverflow.com/questions/35793672/use-unique-ptr-with-glfwwindow
		if (window == nullptr)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			return;
		}
		glfwMakeContextCurrent(window.get());

		this->camera = pMyCamera(new MyCamera());

		glewExperimental = GL_TRUE; //使glew更现代化的处理OpenGL的功能
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return;
		}
		glfwGetFramebufferSize(window.get(), &width, &height); //获取窗口大小
		glViewport(0, 0, width, height); //OpenGL渲染结果在窗口坐标中的位置
		glfwSetKeyCallback(window.get(), key_callback); //按键事件回调函数
		glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window.get(), mouse_callback);
		glfwSetMouseButtonCallback(window.get(), mouseButton_callback);
		glfwSetScrollCallback(window.get(), scroll_callback);


		LoadShaders();

		#ifndef EXPORTDLL
 		LoadModel(); //单独的exe直接导入模型，加上启动器后通过启动器导入
		#endif	

		LoadSkyBox();

		standardCube = pSimpleObject(new SimpleObject(Cubevertices, 36, GetWhiteShader(), GL_LINE_STRIP));
		standardSphere = pSimpleObject(new SimpleObject(SphereVertices, 114, GetWhiteShader(), GL_TRIANGLES));
		standardSphere->AddEBO(SphereIndices, 224);
		
		enablePostEffect = false;

	}

	glm::vec3 aiVec2glmVec(const aiVector3D vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	void ProcessNode(const aiNode* node,const aiScene* scene,const std::string directory,pModel model) //对模型
	{
		for (int i = 0; i < node->mNumMeshes; i++)
		{
			const aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
			std::vector<Vertex> vertices;
			std::vector<int> indices;
			
			for (GLuint x = 0; x < aimesh->mNumVertices; x++)
			{
				vertices.push_back(Vertex( aiVec2glmVec(aimesh->mVertices[x]), aiVec2glmVec(aimesh->mNormals[x]), 
					aiVec2glmVec(aimesh->mTextureCoords[0][x]),aiVec2glmVec(aimesh->mTangents[x]), aiVec2glmVec(aimesh->mBitangents[x])));

			}
			for (GLuint x = 0; x < aimesh->mNumFaces; x++)
			{
				aiFace face = aimesh->mFaces[x];
				for (GLuint j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			pMaterial defaultMaterial = pMaterial(
				new Material(scene->mMaterials[aimesh->mMaterialIndex],directory,shaderArray[0],"defaulMat"+std::to_string(aimesh->mMaterialIndex) )
				);
			MaterialArray.push_back(defaultMaterial);

			pMesh mesh = pMesh(new Mesh(vertices, indices));
			mesh->SetMaterial(defaultMaterial);
			model->meshes.push_back(std::move(mesh));
			
		}
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->ProcessNode(node->mChildren[i], scene, directory, model);;
		}
	}

	void ImportModelFile(std::string path)
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace  );
		
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		std::string directory = path.substr(0, path.find_last_of('\\'));

		const aiNode* root = scene->mRootNode;
		modelPool.push_back(pModel(new Model()));
		ProcessNode(root,scene,directory, modelPool[modelPool.size()-1]);
		modelPool[modelPool.size() - 1]->SetupOBB();
		
		std::cout << "完成模型载入" << std::endl;
	}

	void importShaderFile(std::string Vertpath,std::string FragPath,ShaderType  type)
	{
		switch (type)
		{
		case ShaderType::ObjectShader:
			shaderArray.push_back(pMyShader(new MyShader(Vertpath, FragPath, type)));
			break;
		case ShaderType::postEffectShader:
			shaderArray.push_back(pMyShader(new MyShader(Vertpath, FragPath, type)));
			break;
		case ShaderType::WhiteShader:
			shaderArray.push_back(pMyShader(new MyShader(Vertpath, FragPath, type)));
			break;
		}
		
	}

	void Display()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
		float currentTime = 0;
		while (!glfwWindowShouldClose(window.get() ) )
		{
			if(controller.isCloseWindow) glfwSetWindowShouldClose(window.get(), GL_TRUE);

			

			float deltaTime = glfwGetTime() - currentTime;
			currentTime = glfwGetTime();
			controller.isMouseMoving = false;
			glfwPollEvents();
			if (!controller.isMouseMoving)
			{
				controller.deltaX = 0;
				controller.deltaY = 0;
			}
			camera->Move(deltaTime*controller.speed*glm::vec3((controller.left-controller.right)*camera->Left()+ (controller.forward - controller.backward)*camera->Forward() ));
			camera->RotateTo(glm::vec3(cos(glm::radians(controller.pitch)) * sin(glm::radians(-controller.yaw)),
				sin(glm::radians(controller.pitch)),
				cos(glm::radians(controller.pitch)) * cos(glm::radians(-controller.yaw)))
			,glm::vec3(0,1,0));

 			if (controller.isCastRay)
			{
				RayInfo ri = RayInfo(camera->GetPosition(), camera->Forward(), 200);
				hitInfo hit = RayCast(ri, this->modelPool);
				if (hit.isHit)SelectingModel = hit.hitModel;
				else SelectingModel = nullptr;
			}
			if (SelectingModel != nullptr && controller.isHoldingObject)
			{
				glm::vec3 s = SelectingModel->GetPosition();

				SelectingModel->SetScale(glm::vec3(controller.scaleParameter, controller.scaleParameter, controller.scaleParameter));

				SelectingModel->RotateTo(camera->Forward(),camera->Up());
				SelectingModel->Move( camera->GetPosition() - SelectingModel->GetPosition() +controller.holdingDistance*camera->Forward() );
				
				glm::vec3 dv = camera->GetPosition() - SelectingModel->GetPosition();
				glm::mat4 t = SelectingModel->TRS();
			}
			

			long totalVertexCount = 0;
			for (pModel model : modelPool)
			{
				for (pMesh mesh : model->meshes)
				{
					totalVertexCount += mesh->vertices.size();
				}
			}

			#ifdef EXPORTDLL
			std::string fps;
			sprintf(&fps[0], "%ld", totalVertexCount);
			pipeClient::Send(pipeClient::hWritePipe, &fps[0]);
			#endif 

			//Set Shader
			for (int i = 0; i < MaterialArray.size(); i++)
			{
				if (controller.currentShader < shaderArray.size())
				{
					MaterialArray[i]->shader = shaderArray[controller.currentShader];
				}
			}

			MessageLock.lock();
			switch (PipeMessage)
			{
			case PipeMessageType::changeSkybox:
				skybox = pSkybox(new Skybox(pipeParamater));
				PipeMessage = PipeMessageType::None;
				break;
			case PipeMessageType::importModel:
				ImportModelFile(pipeParamater);
				PipeMessage = PipeMessageType::None;
				break;
			case PipeMessageType::Exit:
				controller.isCloseWindow = true;
				PipeMessage = PipeMessageType::None;
				break;
			case PipeMessageType::None:
				break;
			default:
				break;
			}
			MessageLock.unlock();

			if (controller.isEnablePostEffect) enablePostEffect = true;
			else enablePostEffect = false;

			Render();

		}
	}

	void Render()
	{
		mat_W2V = glm::lookAt(camera->GetPosition(), camera->GetPosition() + camera->Forward(), camera->Up());
		mat_V2P = glm::perspective(camera->FOV.y, (GLfloat)window_width / window_height, camera->camNear, camera->camFar);

		for (GLuint i = 0; i < shaderArray.size(); i++) //向shader加载摄像机位置和光源
		{
			GLShaderObject shader = shaderArray[i]->GLshader;
			glUseProgram(shader);

			GLLocation Vpos_loc = glGetUniformLocation(shader, "viewPos");
			glUniform3fv(Vpos_loc, 1, glm::value_ptr(camera->GetPosition()) );

			GLLocation LightPos_Loc = glGetUniformLocation(shader, "pointLights[0].position");
			GLLocation LightColor_Loc = glGetUniformLocation(shader, "pointLights[0].color");
			glm::vec3 Lightpos = camera->GetPosition();
			glm::vec3 LightColor = glm::vec3(1, 1, 1);
			glUniform3fv(LightPos_Loc, 1, glm::value_ptr(camera->GetPosition()));
			glUniform3fv(LightColor_Loc, 1, glm::value_ptr(LightColor));
			glUseProgram(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (enablePostEffect)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, postEffect_fbo);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//standardSphere.Draw(mat_W2V,mat_V2P);
		for (int i = 0; i < modelPool.size(); i++)
		{
			modelPool[i]->Draw(mat_W2V, mat_V2P);
		}
		skybox->DrawSkybox(mat_W2V,mat_V2P);
		if (SelectingModel!=nullptr)
		{
			standardCube->SetTransMatrix(SelectingModel->TRS() * SelectingModel->OBB);
			standardCube->Draw(mat_W2V, mat_V2P);
		}
		
		if (enablePostEffect)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);
			PostEffectCanvas->Draw(glm::mat4(), glm::mat4());
			glBindFramebuffer(GL_FRAMEBUFFER, postEffect_fbo);
			glEnable(GL_DEPTH_TEST);
		}
		glfwSwapBuffers(window.get());
	}
	
	void LoadSkyBox()
	{
		skybox = pSkybox(new Skybox(".\\skybox"));
	}

	~Browser()
	{
		glDeleteBuffers(1, &postEffect_fbo);
		glDeleteBuffers(1, &postEffect_rbo);
	}
};
