#include <stdio.h>
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>


namespace pipeClient
{

	//using MessageProc = void(*)(char* message) ;
	typedef void (*MessageProc)(char*);

	unsigned long __stdcall Listen(void * pParam);

	MessageProc messageProc = NULL;
	LPTSTR WrtiePipeName ;
	LPTSTR ReadPipeName ;

	HANDLE hWritePipe, hReadPipe;
	DWORD threadId;
	HANDLE hThread = NULL;
	BOOL Finished;

	char writeBuf[100];
	char readBuf[100];
	DWORD cbWritten;
	DWORD dwBytesToWrite = (DWORD)strlen(writeBuf);
	BOOL isInit;

	void initPipe(MessageProc msgProc, LPSTR WPipeName,LPSTR RPipeName)
	{
		messageProc = msgProc;
		WrtiePipeName = WPipeName;
		ReadPipeName = RPipeName;

		hWritePipe = CreateFile(WrtiePipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		hReadPipe = CreateFile(ReadPipeName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if ((hWritePipe == NULL || hWritePipe == INVALID_HANDLE_VALUE) || (hReadPipe == NULL || hReadPipe == INVALID_HANDLE_VALUE))
		{
			printf("Could not open the pipe  - (error %d)\n", GetLastError());
			return;
		}
		else
		{
			printf("PipeConnected\n");
			hThread = CreateThread(NULL, 0, &Listen, NULL, 0, NULL);
		}
		isInit = true;
	}
	int Count = 0;
	void Send(HANDLE pipe,const char* Pstr)
	{
		strcpy(writeBuf, Pstr);
		dwBytesToWrite = strlen(writeBuf);
		dwBytesToWrite = 100;
		WriteFile(hWritePipe, writeBuf, dwBytesToWrite, &cbWritten, NULL);
		printf("WriteByte:%d %d \n ", cbWritten,Count++);
		memset(writeBuf, 0xCC, 100);
	}

	void ClosePipe()
	{
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		Finished = TRUE;
	}


	unsigned long __stdcall Listen(void * pParam) {
		BOOL fSuccess;
		
		DWORD cbRead;
		DWORD dwBytesToRead = sizeof(readBuf);
		while (1)
		{
			fSuccess = ReadFile(hReadPipe, readBuf, dwBytesToRead, &cbRead, NULL);
			
 			if (fSuccess)
			{
				if (messageProc != NULL)
				{
					messageProc(readBuf);
				}
				else
				{
					printf("withou message proc");
				}
			}
			if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
			{
				printf("Can't Read\n");
				if (Finished)
					break;
			}
		}
		printf("EndListen");
		return 0;
	}
}

