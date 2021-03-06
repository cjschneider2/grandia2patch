#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <exception>

/*
	Simply a laucher for the Grandia2 executable.
	Injects the DLL that is responsible for doing the patching job.
*/

int main()
{
	const char* dllName = "Grandia2-Dll.dll";
	const char* exeName = "Grandia2.exe";

	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory ( &startupInfo, sizeof (startupInfo) );
	startupInfo.cb = sizeof (startupInfo);
	ZeroMemory ( &processInfo, sizeof(processInfo) );

	HRESULT result = CreateProcessA (NULL, (LPSTR) exeName, NULL, NULL, 
		FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo );

	if (result == 0)
	{
		throw std::exception ("Failed to create process");
	}

	int processID = processInfo.dwProcessId;
	HANDLE process = OpenProcess (PROCESS_ALL_ACCESS, FALSE, processID);

	if (process == NULL) {
		throw std::exception("Failed to open process");
	}

	void * address = (void *) GetProcAddress
		(GetModuleHandle (L"kernel32.dll"), "LoadLibraryA");
	
	void * allocatedMemory = (void *) VirtualAllocEx (process, NULL, strlen(dllName), 
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (allocatedMemory == NULL) {
		throw std::exception("Failed to create allocate memory in process");
	}

	if ( WriteProcessMemory (process, allocatedMemory, dllName, strlen(dllName), NULL) == 0 )
		throw std::exception("Failed to write process memory");

	HANDLE threadID = CreateRemoteThread (process, NULL, 0, 
		(LPTHREAD_START_ROUTINE) address, allocatedMemory, NULL, NULL);

	if (threadID == NULL) {
		throw std::exception("Failed to create remote thread");
	}

	Sleep(250);
	ResumeThread(processInfo.hThread);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
	
	return 0;
}

