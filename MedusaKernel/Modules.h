#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>


struct UserModule
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	WCHAR Path[260];
};

class Modules
{
public:
	Modules() = default;
	~Modules() = default;
public:
	std::vector<UserModule> GetWin32MoudleList(ULONG64 PID);
	bool R0MapInject(ULONG64 PID,ULONG64 Size,void *DLLImage);
private:

};

