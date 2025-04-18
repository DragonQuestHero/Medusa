#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>


#include <QString>


#include "ntdll.h"

struct PROCESS_LIST
{
	ULONG64 PID;
	bool Check;
	WCHAR Name[MAX_PATH];
	WCHAR Path[MAX_PATH];
	ULONG64 EPROCESS;
};

struct ModuleList
{
	WCHAR Name[MAX_PATH];
	WCHAR Path[MAX_PATH];
};

class Process
{
public:
	Process() = default;
	~Process() = default;
public:
	bool GetProcessList(bool);
	bool GetProcessListR3();
	bool GetProcessListFromR0();
	bool QueryValue(const std::wstring& valueName, const LPCWSTR& szModuleName, std::wstring& retStr);
	bool KillProcess(int mode, ULONG64 pid);
private:
	std::vector<ModuleList> GetModuleListR3(ULONG64 PID);
public:
	std::vector<PROCESS_LIST> _Process_List_R3;
	std::vector<PROCESS_LIST> _Process_List_R0;
private:
	
};

