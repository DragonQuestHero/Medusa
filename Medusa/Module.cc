#include "Module.h"





std::vector<MODULEENTRY32W> Module::GetWin32MoudleList(ULONG64 PID)
{
	std::vector<MODULEENTRY32W> temp_vector;

	HANDLE        hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		int a = GetLastError();
		return temp_vector;
	}

	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return temp_vector;
	}

	do {
		temp_vector.push_back(me32);

	} while (Module32Next(hModuleSnap, &me32));
	CloseHandle(hModuleSnap);
	return temp_vector;
}