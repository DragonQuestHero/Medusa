#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>

#include <QString>


#include "ntdll.h"


class Module
{
public:
	Module() = default;
	~Module() = default;
public:
	std::vector<MODULEENTRY32W> GetWin32MoudleList(ULONG64 PID);
private:

};

