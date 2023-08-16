#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>

#include <QString>


#include "ntdll.h"


struct _CheckDifferent
{
	WCHAR Name[MAX_PATH];
	WCHAR Path[MAX_PATH];
	ULONG64 Addr;
	char FileHex[20];
	char MemoryHex[20];
};


class FileCheck
{
public:
	FileCheck() = default;
	~FileCheck() = default;
public:
	bool CheckSimple(ULONG64 PID);
	std::vector<_CheckDifferent> CheckPlain(ULONG64 PID);
private:


};



