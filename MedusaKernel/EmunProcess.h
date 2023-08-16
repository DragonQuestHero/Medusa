#pragma once
#include <ntifs.h>
#include <intrin.h>

#include <vector>


struct PROCESS_LIST
{
	ULONG64 PID;
	bool Check;
	WCHAR Name[260];
	WCHAR Path[260];
};


class EmunProcess
{
public:
	EmunProcess() = default;
	~EmunProcess() = default;
public:
	bool EmunProcessALL();
	bool EmunProcessUseingNT();
	bool EmunProcessUseingPID();
public:
	bool Get_Process_Image(HANDLE Process_Handle, UNICODE_STRING* Process_Path);
public:
	std::vector<PROCESS_LIST> _Process_List;
private:

};

