// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Windows.h"
#include "DefineTableService.h" // макро для TableService
#include "PrototypeService.h"  // прототипы  обслуживающих потоков 

BEGIN_TABLESERVICE              // таблица 
ENTRYSERVICE("Echo", EchoServer),
ENTRYSERVICE("Time", TimeServer),
ENTRYSERVICE("0001", ServiceServer01)
END_TABLESERVICE;

extern "C" __declspec(dllexport) HANDLE  SSS(char* id, LPVOID prm)
{
	
	HANDLE rc = NULL;
	int  i = 0;
	while (i < SIZETS && strcmp(TABLESERVICE_ID(i), id) != 0)i++;
	if (i < SIZETS)
		rc = CreateThread(NULL, NULL,
			TABLESERVICE_FN(i), prm, NULL, NULL);
	return rc;
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	//	printf("DLL_PROCESS_ATTACH\n");
		break;
	case DLL_THREAD_ATTACH:
	//	printf("DLL_THREAD_ATTACH\n");
		break;
	case DLL_THREAD_DETACH:
		//printf("DLL_THREAD_DETACH\n");
		break;
	case DLL_PROCESS_DETACH:
	//	printf("DLL_PROCESS_DETACH\n");
		break;
		
	}
	return TRUE;
}

