#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>


struct ThreadList
{
	ULONG64 TID;
	ULONG64 ETHREAD;
	ULONG64 StartAddr;
	WCHAR Name[260];
};

class Threads
{
public:
	Threads() = default;
	~Threads() = default;
public:
	bool InitWin32StartAddressOffset();
	std::vector<ThreadList> GetThreadListR0(ULONG64 PID);
private:
	ULONG64 Win32StartAddressOffset;
};
