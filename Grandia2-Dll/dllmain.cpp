// dllmain.cpp: definisce il punto di ingresso per l'applicazione DLL.
#pragma once
#include "stdafx.h"
#include "dllmain.h"
#include "HookDescriptor.h"
#include "Logger.h"




HookDescriptor<type_SetDisplayMode> hook_SetDisplayMode;
HookDescriptor<type_CreateWindowEx> hook_CreateWindowEx;
HookDescriptor<type_DirectDrawCreate> hook_DirectDrawCreate;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH:
		InitHooks();
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

HRESULT InitHooks()
{
	Logger::Instance()->SetLogFile("Log.txt");


	HMODULE handle_User32 = LoadLibraryW (L"user32.dll");
	void * handle_CreateWindowEx = ( void * ) 
		GetProcAddress( handle_User32,	"CreateWindowEx");

	HMODULE handle_DirectDraw = LoadLibraryW (L"ddraw.dll");
	void * handle_DirectDrawCreate = ( void * ) 
		GetProcAddress( handle_DirectDraw, "DirectDrawCreate");
	

	hook_CreateWindowEx = HookDescriptor< type_CreateWindowEx > :: CreateHook< type_CreateWindowEx >
		( (void *) handle_CreateWindowEx, (void *) ProxyCreateWindowEx);
	
	hook_DirectDrawCreate = HookDescriptor< type_DirectDrawCreate >::CreateHook< type_DirectDrawCreate >
		( (void *) handle_DirectDrawCreate, (void *) ProxyDirectDrawCreate);
	
	return S_OK;
}


HWND __stdcall ProxyCreateWindowEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, 
	DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParent, 
	HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)

{
	hook_CreateWindowEx.UnsetHook();
	HWND hwnd = hook_CreateWindowEx.originalFunction(dwExStyle, lpClassName, lpWindowName, 
		WS_POPUP, x, y, nWidth, nHeight, hParent, hMenu, hInstance, lpParam);
	hook_CreateWindowEx.SetHook();
	return hwnd;

}

HRESULT __stdcall ProxyDirectDrawCreate(GUID * guid, IDirectDraw ** directDrawObject, IUnknown * pUnknown)
{
	hook_DirectDrawCreate.UnsetHook ();
	int result = hook_DirectDrawCreate.originalFunction (guid, directDrawObject, pUnknown );	
	hook_DirectDrawCreate.SetHook ();
	
	// set com hook
	return result;

}

HRESULT __stdcall ProxySetDisplayMode(IDirectDraw * _this , DWORD width, DWORD height, DWORD bpp, DWORD refresh, DWORD flags)
{
	HMODULE hModule = GetModuleHandle(L"user32.dll");
	typedef DWORD (__stdcall* type_GetSystemMetrics)(int);
	type_GetSystemMetrics handle_GetSystemMetrics = 
		(type_GetSystemMetrics) GetProcAddress(hModule, "GetSystemMetrics");

	hook_SetDisplayMode.UnsetHook();
	hook_SetDisplayMode.originalFunction (_this,
		handle_GetSystemMetrics(SM_CXSCREEN), handle_GetSystemMetrics(SM_CYSCREEN), 
		bpp, refresh, flags);
	hook_SetDisplayMode.SetHook();

	return S_OK;
}
