#include "Test.h"
#include "MemoryRW.h"
#include "Threads.h"
#include "KernelModules.h"
#include "EmunProcess.h"

#include <string>

void TestWalkStack()
{
	DbgBreakPoint();

	Threads _Threads;
	_Threads.InitWin32StartAddressOffset();

	EmunProcess _EmunProcess;
	_EmunProcess.EmunProcessALL();
	for (auto x : _EmunProcess._Process_List)
	{
		if (x.Name == std::wstring(L"dwm.exe"))
		{
			std::vector<ThreadList> temp_vector = _Threads.GetThreadListR0(x.PID);
			for (auto y : temp_vector)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TID==%d\n", y.TID);
				std::vector<ULONG64> temp_walk_vector = _Threads.StackWalkThreadUser(y.TID);
				for (auto n : temp_walk_vector)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", n);
				}
			}
		}
	}
	
	
	/*PETHREAD tempthd;
	NTSTATUS status;
	for (int i = 1000; i < 65535; i = i + 4)
	{
		status = PsLookupThreadByThreadId((HANDLE)i, &tempthd);
		if (NT_SUCCESS(status))
		{
			_Threads.StackWalkThread(i);
			break;
		}
	}*/
}

void TestReadKernelMemory()
{
	DbgBreakPoint();
	ULONG64 temp = 0;
	KernelSafeReadMemoryIPI((ULONG64)&TestReadKernelMemory, &temp, 8);
	int a = 1;
}

void TestGetKernel()
{
	KernelModules _KernelModules;
	DbgBreakPoint();
	_KernelModules.GetKernelModuleList4();
}