#pragma once
#include <ntifs.h>
#include <intrin.h>

#include "CRT/NtSysAPI_Func.hpp"

#include <string>

struct Message_NtReadWriteVirtualMemory
{
	HANDLE ProcessId;
	HANDLE ProcessHandle;
	PVOID BaseAddress;
	PVOID Buffer;
	SIZE_T BufferBytes;
	PSIZE_T ReturnBytes;
	bool Read;
};

NTSTATUS NewNtReadWriteVirtualMemory(Message_NtReadWriteVirtualMemory* message);
bool KernelSafeReadMemoryIPI(ULONG64 addr, void* Buffer, ULONG64 Size);
bool KernelSafeReadMemoryDPC(ULONG64 addr, void* Buffer, ULONG64 Size);