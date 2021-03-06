#pragma once
#include "stdafx.h"
#include "dllmain.h"
#include "HookDescriptor.h"

/*
	This DLL gets injected into the target process before the execution begins.
	Sets the API Hooks that are necessary to fix the game.
*/

HookDescriptor<type_CreateWindowEx> hook_CreateWindowEx;
HookDescriptor<type_GetLogicalDrives> hook_GetLogicalDrives;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH:
		InitHooks ();
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

HRESULT InitHooks ()
{
	HMODULE handle_User32 = LoadLibraryW (L"user32.dll");
	void * handle_CreateWindowEx = (void *) 
		GetProcAddress (handle_User32,	"CreateWindowExA"); 


	HMODULE handle_Kernel32 = LoadLibraryW (L"kernel32.dll");
	void * handle_GetLogicalDrives = (void *)
		GetProcAddress (handle_Kernel32, "GetLogicalDrives");
		

	hook_CreateWindowEx = HookDescriptor< type_CreateWindowEx > :: CreateHook< type_CreateWindowEx >
		( (void *) handle_CreateWindowEx, (void *) ProxyCreateWindowEx);

	hook_GetLogicalDrives = HookDescriptor< type_GetLogicalDrives > :: CreateHook< type_GetLogicalDrives >
		( (void *) handle_GetLogicalDrives, (void *) ProxyGetLogicalDrives);
	
	return S_OK;
}

/*
	Proxy function for the CreateWindowEx API call.
	It just changes the dwStyle parameter in order to make the window borderless in modern systems.
*/

HWND __stdcall ProxyCreateWindowEx (DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, 
	DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParent, 
	HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)

{

	hook_CreateWindowEx.UnsetHook ();
	HWND window = hook_CreateWindowEx.originalFunction (dwExStyle, lpClassName, lpWindowName, 
		WS_POPUP, x, y, nWidth, nHeight, hParent, hMenu, hInstance, lpParam);
	hook_CreateWindowEx.SetHook ();
	return window;

}

/*
	Proxy function for GetLogicalDrives. 
	Runtime code patching is done here, since the function gets executed early in the executable.
*/

DWORD _stdcall ProxyGetLogicalDrives ()
{
	PatchNoCD ();
	ResolutionPatch ();

	hook_GetLogicalDrives.UnsetHook ();
	return hook_GetLogicalDrives.originalFunction ();
}

/*
	Patches the jumper that stops the execution if the CD is not in tray.
*/

void PatchNoCD ()
{
	void * cdJumpAddress = (void *) 0x559619;
	unsigned char jumpByte = 0xEB;
	WriteByte (cdJumpAddress, &jumpByte);
}

/*
	Patches the resolution selection jump table.
*/

void ResolutionPatch ()
{
	typedef DWORD (_stdcall* type_GetSystemMetrics) (int);

	HMODULE hModule = GetModuleHandle (L"user32.dll");
	type_GetSystemMetrics handle_GetSystemMetrics =
		(type_GetSystemMetrics) GetProcAddress (hModule, "GetSystemMetrics");
	
	unsigned short screenWidth = handle_GetSystemMetrics (SM_CXSCREEN);
	unsigned short screenHeight = handle_GetSystemMetrics (SM_CYSCREEN);
	unsigned char bitDepth = 16;

	WriteByte ( (void *) 0x40D496, &bitDepth);
	WriteWord ( (void *) 0x40D4B2, &screenWidth);
	WriteWord ( (void *) 0x40D4BC, &screenHeight);

	WriteByte ( (void *) 0x40D4CE, &bitDepth);
	WriteWord ( (void *) 0x40D4D8, &screenWidth);
	WriteWord ( (void *) 0x40D4E2, &screenHeight);

	WriteWord ( (void *) 0x40D4FB, &screenWidth);
	WriteWord ( (void *) 0x40D505, &screenHeight);

	WriteByte ( (void *) 0x40D514, &bitDepth);
	WriteWord ( (void *) 0x40D51F, &screenWidth);
	WriteWord ( (void *) 0x40D528, &screenHeight);

	WriteWord ( (void *) 0x40D541, &screenWidth);
	WriteWord ( (void *) 0x40D54B, &screenHeight);

}

void WriteByte (void * destination, unsigned char * byte)
{
	DWORD oldMemoryProtect;
	VirtualProtect (destination, 1, PAGE_EXECUTE_READWRITE, &oldMemoryProtect);
	memcpy (destination, byte, 1);
	DWORD tempMemoryProtect;
	VirtualProtect (destination, 1, oldMemoryProtect, &tempMemoryProtect);
}

void WriteWord (void * destination, unsigned short * word)
{
	DWORD oldMemoryProtect;
	VirtualProtect (destination, 2, PAGE_EXECUTE_READWRITE, &oldMemoryProtect);
	memcpy (destination, word, 2);
	DWORD tempMemoryProtect;
	VirtualProtect (destination, 2, oldMemoryProtect, &tempMemoryProtect);
}
