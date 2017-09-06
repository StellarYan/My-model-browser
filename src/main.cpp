#define _CRT_SECURE_NO_WARNINGS


//#define EXPORTDLL

#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include "PipeClient.h"

enum PipeMessageType
{
	importModel, changeSkybox, Exit, None
};

std::string pipeParamater;
std::mutex MessageLock;
PipeMessageType PipeMessage = PipeMessageType::None;


#include "MyBrowser.h"



void msgpro(char* str)
{
	printf("%c", str[0]);
	std::string s(str);
	if (s.size() < 4)
	{
		return;
	}
	std::string command(&s[0], &s[4]);
	std::string paramater(&s[4], &s[0]+s.size() );
	
	MessageLock.lock();
	if (command=="Exit") //退出
	{
		PipeMessage = PipeMessageType::Exit;
	}
	else if (command == "impM") //导入模型
	{
		pipeParamater = paramater;
		PipeMessage = PipeMessageType::importModel;
	}
	else if (command == "Csky") //改变天空盒
	{
		pipeParamater = paramater;
		PipeMessage = PipeMessageType::changeSkybox;
	}
	else
	{

	}
	MessageLock.unlock();
	
}

#ifdef EXPORTDLL
extern "C"
{
	__declspec(dllexport) void __stdcall InitBrowser()
	{
		Browser* pB;
		pB = new Browser("my window", 800, 600);
		pB->camera->SetPosition(glm::vec3(0.0f, 12.0f, 18.0f));
		pB->camera->RotateTo(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		pipeClient::initPipe(msgpro, TEXT("\\\\.\\pipe\\GL2GUI_PIPE"), TEXT("\\\\.\\pipe\\GUI2GL_PIPE"));
		pB->EnablePostEffect();
		
		pB->Display();

		delete pB;
		pipeClient::ClosePipe();
		glfwTerminate();
		system("Pause");
	}

	__declspec(dllexport) void __stdcall PipeCommand() 
	{
		
	}
	
}
#endif 
#ifndef EXPORTDLL

int main()
{

	Browser* pB = new Browser("my window", 800, 600);
	pB->camera->SetPosition(glm::vec3(0.0f, 12.0f, 18.0f));
	pB->camera->RotateTo(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	pipeClient::initPipe(msgpro, TEXT("\\\\.\\pipe\\GL2GUI_PIPE"), TEXT("\\\\.\\pipe\\GUI2GL_PIPE"));
	pB->EnablePostEffect();
	pB->ImportModelFile(".\\Models\\nano\\nanosuit.obj");
	pB->Display();

	delete pB;
	pipeClient::ClosePipe();
	glfwTerminate();
	system("Pause");
}

#endif // !EXPORTDLL
