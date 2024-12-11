#pragma once
#include <ntifs.h>
#include <intrin.h>

#include "CRT/NtSysAPI_Func.hpp"

#include <string>
#include <vector>

#include "PageTable.h"

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

typedef union _ADDRESS_STRUCTURE {
	struct {
		ULONG64 Offset : 12;         // 11:0��ҳ��ƫ��
		ULONG64 PEIndex : 9;         // 20:12��ҳ������
		ULONG64 PDIndex : 9;         // 29:21��ҳĿ¼����
		ULONG64 PDPTIndex : 9;       // 38:30��ҳĿ¼ָ�������
		ULONG64 PML4Index : 9;      // 47:39��PML4 ����
		ULONG64 Reserved : 16;      // 63:48������λ

	} Bits;
	uint64_t BitAddress;
} ADDRESS_STRUCTURE;


using UserMemoryListStructCR3 = struct
{
	ULONG64 Addr;
	ULONG64 Size;
	HardwarePteX64 PteX64;
};

NTSTATUS NewNtReadWriteVirtualMemory(Message_NtReadWriteVirtualMemory* message);
bool ReadKernelMemory(ULONG64 addr, void* Buffer, ULONG64 Size);
bool KernelSafeReadMemoryIPI(ULONG64 addr, void* Buffer, ULONG64 Size);
bool KernelSafeReadMemoryDPC(ULONG64 addr, void* Buffer, ULONG64 Size);
std::vector<UserMemoryListStructCR3> ScannUserMemoryFromCR3(ULONG64 PID);
bool KernelReadPhysicalMemory(ULONG64 addr, void* Buffer, ULONG64 Size);
bool KernelReadSpecialPhysicalMemory(ULONG64 addr, void* Buffer, ULONG64 Size);