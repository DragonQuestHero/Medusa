#pragma once
#include <ntifs.h>
#include <intrin.h>

#include <vector>
#include <set>


struct PROCESS_LIST
{
	ULONG64 PID;
	bool Check;
	WCHAR Name[260];
	WCHAR Path[260];
	ULONG64 EPROCESS;
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
	void EmunProcessSecondCheck();
public:
	bool Get_Process_Image(HANDLE Process_Handle, UNICODE_STRING* Process_Path);
public:
	std::vector<PROCESS_LIST> _Process_List;
	std::set<ULONG64> _ProcessSecondCheckList;
private:

};

